// main.cpp
// MEDHASH: Medical Record Hashing System
// All patient data used by this program is synthetic.
//
// ======================= REQUIREMENTS MAP =======================
// Search the code for "[REQ §" to find each tagged location.
//
// §5.1 Linear Probing ............. LinearProbingHash.h  (probeIndex)
// §5.2 Quadratic Probing .......... QuadraticProbingHash.h (probeIndex),
//                                   failure detection in OpenAddressHash.h (insert)
// §5.3 Double Hashing ............. DoubleHash.h (secondaryHash, probeIndex)
// §5.1-5.3 shared open addressing . OpenAddressHash.h (hash, tryInsert, findIndex,
//                                   DELETED handling in remove)
// §5.4 Separate Chaining .......... SeparateChainingHash.h, ChainNode.h
// §5.5 Traditional Bucket Hashing . BucketHash.h (homeSlot, slotFor)
// §5.6 Alternate Bucket Hashing ... AlternateBucketHash.h (homeSlot)
// §6   Overflow Area .............. BucketHash.h (insert, search, remove)
// §7   Load Factor ................ HashTableBase.h (getLoadFactor), every insert()
// §7   Rehashing .................. rehash() in OpenAddressHash.h,
//                                   SeparateChainingHash.h, BucketHash.h
// §11  Hash Statistics ............ HashStatistics.h/.cpp, getStatistics()
// §12  Compare All / Performance .. MedicalHashApplication.cpp (runComparison)
// =================================================================

#include "MedicalHashApplication.h"
#include "InputEnded.h"
#include <iostream>

int main()
{
    MedicalHashApplication app;
    try
    {
        app.run();
    }
    catch (const InputEnded&)
    {
        std::cout << "\nInput ended. Exiting.\n";
    }
    return 0;
}
