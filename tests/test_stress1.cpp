#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include "thread.h"

void servicer(void *);
void requester(void *);
void printQueue();
void print_request(size_t, size_t);
void print_service(size_t, size_t);

class Lock
{
   mutex &m;
   public:
   Lock(mutex &m_) : m(m_) {  m.lock();  }
   ~Lock() {   m.unlock();  }
};

struct Request
{
   int track;
   int threadNum;
};

struct RequesterData
{
   std::vector<int> input;
   int threadNum;
};

struct Monitor
{
	mutex queueM;
	size_t maxDiskQueue;
   size_t numThreads;
	std::vector<Request> diskQueue;
   std::vector<bool> threadHasRequested;
   std::vector<bool> threadHasFinished;
};

Monitor monitor;
cv queueFull;
cv requestMade;
int numInputs = 5;

int main(int argc, char *argv[])
{
	// Read in file names
   std::vector<std::vector<int>> inputs;
   std::vector<int> v1 = { 566,
999,
623,
167,
766,
977,
106,
826,
839,
654,
983,
317,
560,
762,
136,
658,
78,
711,
253,
74,
818,
117,
849,
975,
675,
932,
604,
230,
301,
237,
533,
333,
803,
235,
486,
447,
69,
397,
643,
809,
196,
243,
423,
201,
585,
886,
201,
585,
547,
511,
314,
538,
664,
960,
678,
873,
834,
906,
723,
44,
195,
953,
215,
301,
561,
745,
84,
991,
6,
216 };
std::vector<int> v2 = { 620,
793,
717,
842,
706,
962,
883,
340,
464,
736,
905,
628,
842,
467,
858,
827,
564,
844,
432,
277,
418,
202,
605,
965,
203,
386,
599,
684,
805,
283,
51,
572,
32,
920,
493,
866,
820,
183,
456,
873,
951,
391,
912,
272,
15,
868,
760,
42,
610,
171,
263,
288,
228,
3,
538,
420,
535,
94,
567,
511,
786,
325,
789,
194,
455,
447,
721,
308,
705,
42 };

std::vector<int> v3 = {555,
636,
259,
319,
596,
881,
190,
551,
189,
399,
876,
236,
370,
602,
966,
951,
147,
635,
50,
126,
146,
394,
110,
532,
700,
525,
351,
805,
161,
506,
345,
929,
592,
268,
457,
414,
953,
128,
665,
582,
800,
751,
641,
580,
645,
540,
628,
37,
740,
983,
142,
926,
923,
214,
866,
737,
334,
255,
121,
979,
504,
500,
932,
162,
111,
156,
601,
255,
304,
864
};

std::vector<int> v4 = { 197,
218,
407,
545,
84,
307,
171,
37,
780,
773,
228,
444,
730,
709,
11,
547,
903,
168,
68,
519,
68,
475,
988,
417,
618,
875,
603,
342,
134,
290,
315,
429,
640,
232,
829,
90,
835,
548,
527,
757,
269,
185,
694,
856,
424,
690,
961,
13,
386,
578,
605,
287,
439,
289,
828,
402,
436,
534,
45,
638,
744,
114,
932,
787,
677,
3,
46,
804,
284,
393 };

std::vector<int> v5 = {336,
911,
141,
57,
240,
933,
83,
627,
789,
474,
755,
239,
481,
286,
454,
977,
312,
532,
902,
94,
293,
317,
87,
82,
46,
467,
656,
262,
157,
385,
486,
582,
263,
541,
273,
944,
910,
527,
338,
256,
858,
742,
524,
611,
307,
15,
212,
821,
315,
296,
406,
993,
938,
61
};

inputs.push_back(v1);
inputs.push_back(v2);
inputs.push_back(v3);
inputs.push_back(v4);
inputs.push_back(v5);
   // Read in max queue size
   monitor.maxDiskQueue = 3;
   // Start thread for servicer()
   thread_startfunc_t servicer_ptr = (void(*)(void*))servicer;
   cpu::boot(1, servicer_ptr, (void *)&inputs, false, false, 0);
}

void servicer(void *inputData)
{
   Lock queueLock(monitor.queueM);
   std::vector<std::vector<int>> inputs = *((std::vector<std::vector<int>> *)inputData);

   // Build thread data structures
   for (size_t i = 0; i < inputs.size(); ++i)
   {
      RequesterData *data = new RequesterData;
      data->input = inputs[i];
      data->threadNum = (int)i;
      monitor.threadHasRequested.push_back(false);
      monitor.threadHasFinished.push_back(false);
      thread(thread_startfunc_t(requester), (void *) data);
   }

   monitor.numThreads = (int)inputs.size();
   int lastTrack = 0;
   while (monitor.numThreads != 0)
   {
      while ((monitor.numThreads >= monitor.maxDiskQueue &&
              monitor.diskQueue.size() != monitor.maxDiskQueue) ||
             (monitor.numThreads < monitor.maxDiskQueue &&
              monitor.numThreads != monitor.diskQueue.size()))
      {
         queueFull.wait(monitor.queueM);
      }

      auto closestRequest = monitor.diskQueue.begin();
      for (auto it = monitor.diskQueue.begin();
                it < monitor.diskQueue.end(); ++it)
      {
         if (std::abs(lastTrack - it->track) <
            (std::abs(lastTrack - closestRequest->track)))
         {
            closestRequest = it;
         }
      }

      print_service((unsigned int)closestRequest->threadNum,
                    (unsigned int)closestRequest->track);

      if (monitor.threadHasFinished[closestRequest->threadNum])
         --monitor.numThreads;
      lastTrack = closestRequest->track;
      monitor.threadHasRequested[closestRequest->threadNum] = false;
      monitor.diskQueue.erase(closestRequest);

      requestMade.broadcast();
   }
}

void requester(void *threadData)
{
   // Acquire and open file.
   RequesterData *requesterData = (RequesterData*)threadData;
   int threadNum = requesterData->threadNum;
   std::vector<int> trackRequests(*((std::vector<int>*)threadData));
   int track;
   Lock queueLock(monitor.queueM);

   for (size_t i = 0; i < trackRequests.size(); ++i)
   {
      track = trackRequests[i];
      Request request = {track, threadNum};

      while  (monitor.threadHasRequested[threadNum] ||
            ((monitor.numThreads >= monitor.maxDiskQueue &&
              monitor.diskQueue.size() == monitor.maxDiskQueue) ||
             (monitor.numThreads <  monitor.maxDiskQueue &&
              monitor.numThreads == monitor.diskQueue.size())))
      {
         requestMade.wait(monitor.queueM);
      }
      
      monitor.diskQueue.push_back(request);
      monitor.threadHasRequested[threadNum] = true;
      print_request((unsigned int)threadNum,
                    (unsigned int)request.track);
      assert(monitor.diskQueue.size() <= monitor.maxDiskQueue);
      queueFull.signal();
   }
   
   monitor.threadHasFinished[threadNum] = true;
   delete requesterData;
}

void print_service(size_t threadNum, size_t track)
   {
      std::cout << "Service requester " << threadNum << " track " << track << std::endl;
   }

void print_request(size_t threadNum, size_t track)
{
   std::cout << "requester " << threadNum << " track " << track << std::endl;
}

void printQueue()
{
   for (size_t i = 0; i < monitor.diskQueue.size(); ++i )
   {
      std::cout << "Queue item " << i << ": " << "threadNum = "
                << monitor.diskQueue[i].threadNum << " track = "
                << monitor.diskQueue[i].track << '\n';
   }
}
