// testlambdatypes.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <bit>
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

#include <intrin.h>

constexpr const size_t FILE_ALLOCATION_BITMAP_SIZE = 195;

// Algorithm for operating on multi-width ranges in a single buffer.
// For now, assuming 32-bit wide registers.
template < size_t BYTE_RANGE_LENGTH, typename Func>
void ApplyToByteRange(uint8_t* pA, Func&& func) {
   constexpr size_t k32bitElementCount = BYTE_RANGE_LENGTH / 4;
   constexpr size_t kBytesRemainingAfter32 = BYTE_RANGE_LENGTH % 4;
   constexpr size_t k16bitElementCount = kBytesRemainingAfter32 / 2;
   constexpr size_t kBytesRemainingAfter16 = kBytesRemainingAfter32 % 2;


   if constexpr (k32bitElementCount > 0)
   {
      for (size_t count = k32bitElementCount; count > 0; --count)
      {
         func((uint32_t*)pA);
         pA += sizeof(uint32_t);
      }
   }

   if constexpr (k16bitElementCount > 0)
   {
      for (size_t count = k16bitElementCount; count > 0; --count)
      {
         func((uint16_t*)pA);
         pA += sizeof(uint16_t);
      }

   }

   if constexpr (kBytesRemainingAfter16 > 0)
   {
      for (size_t count = kBytesRemainingAfter16; count > 0; --count)
      {
         func(pA);
         ++pA;
      }
   }
}


// Algorithm for operating on multi-width ranges. For now, assuming 32-bit wide
// registers.
template <size_t BYTE_RANGE_LENGTH, typename Func>
void ApplyToByteRanges(uint8_t* pA, uint8_t const* pB, Func&& func) {
   constexpr size_t k32bitElementCount = BYTE_RANGE_LENGTH / 4;
   constexpr size_t kBytesRemainingAfter32 = BYTE_RANGE_LENGTH % 4;
   constexpr size_t k16bitElementCount = kBytesRemainingAfter32 / 2;
   constexpr size_t kBytesRemainingAfter16 = kBytesRemainingAfter32 % 2;


   if constexpr (k32bitElementCount > 0)
   {
      for (size_t count = k32bitElementCount; count > 0; --count)
      {
         func((uint32_t*)pA, (uint32_t*)pB);
         pA += sizeof(uint32_t);
         pB += sizeof(uint32_t);
      }
   }

   if constexpr (k16bitElementCount > 0)
   {
      for (size_t count = k16bitElementCount; count > 0; --count)
      {
         func((uint16_t*)pA, (uint16_t*)pB);
         pA += sizeof(uint16_t);
         pB += sizeof(uint16_t);
      }

   }

   if constexpr (kBytesRemainingAfter16 > 0)
   {
      for (size_t count = kBytesRemainingAfter16; count > 0; --count)
      {
         func(pA, pB);
         ++pA;
         ++pB;
      }
   }
}

static int count = 0;

int main()
{
   std::cout << "Hello World!\n";
   uint8_t bitmap[FILE_ALLOCATION_BITMAP_SIZE + 1];
   uint32_t ix = 0; 
   ApplyToByteRange<FILE_ALLOCATION_BITMAP_SIZE>(bitmap, [&](auto* pA) {
      std::cout << ix << " size: " << sizeof(*pA) << "\n";
      *pA = 0x40;
      ++ix;
      });
   std::cout << "Complete! \n\n";
   
   bitmap[4] = 0x04;
   bitmap[8] = 0x02;
   bitmap[16] = 0x01;
   bitmap[32] = 0x80;
   bitmap[195] = 0x55;
   std::cout << "Bitmap popcount test\n";
   
   uint32_t count = 0;
   ix = 0;
   ApplyToByteRange<FILE_ALLOCATION_BITMAP_SIZE>(bitmap,
      [&](auto const* pA)
      {
         std::cout << "[" << ix << "] ["<< ((size_t)pA - (size_t)bitmap) << "] " << std::hex << (unsigned int)(*pA) << std::dec << " (" << std::popcount(*pA) << " bits) size: " << sizeof(*pA) << "\n";
         count += std::popcount(*pA);
         ++ix;
      }
   );
   std::cout << "Total bits set: " << count << "\n\n";

   std::cout << "Last attempt, but now without output overhead \n\n";
   count = 0;

   auto start = std::chrono::steady_clock::now();
   uint64_t start_cycle = __rdtsc();

   for (int i = 0; i < 1000; ++i)
   {
      ApplyToByteRange<FILE_ALLOCATION_BITMAP_SIZE>(bitmap,
         [&](auto const* pA)
         {
            count += std::popcount(*pA);
         }
      );
   }
   uint64_t end_cycle = __rdtsc();

   // 3. Capture the end time point
   auto end = std::chrono::steady_clock::now();

    //4. Calculate the duration and cast it to nanoseconds
   auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

   std::cout << "Total bits set: " << count << "\n";
   std::cout << "Total Duration: " << duration.count()  << " ns\n";
   std::cout << "Single Iteration Duration: " << ( duration.count() / 1000.0 ) << " ns\n";
   std::cout << "Cycles (RDTSC) " << (double)(end_cycle - start_cycle)/1000.0 << "\n";

   
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
