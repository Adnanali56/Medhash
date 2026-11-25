# MEDHASH: Medical Record Hashing System

A menu-driven C++17 application that indexes synthetic medical records by patient ID and compares six hashing / collision-resolution techniques plus rehashing, using a generic Dictionary ADT, templates, and object-oriented design.

All patient data in this project is synthetic. No real patient information is used.

## Project Links

- YouTube presentation and demo: (coming soon)
- LinkedIn post: (coming soon)

## Build and Run (Ubuntu)

```bash
g++ -std=c++17 -Wall -Iinclude src/*.cpp -o medhash
./medhash
```

Run the program from the project folder so the comparison mode can save its CSV files into `data/`.

## Techniques Implemented

| Technique | Formula / behaviour |
| --- | --- |
| Linear probing | `index = (home + i) % tableSize` |
| Quadratic probing | `index = (home + i*i) % tableSize`; reports failure, never falls back to linear |
| Double hashing | `h1 = key % tableSize`, `h2 = PRIME - (key % PRIME)`, `index = (h1 + i*h2) % tableSize` |
| Separate chaining | Each slot heads a linked chain of `ChainNode`s (no `std::list`) |
| Bucket hashing | `bucket = key % numBuckets`; fill the bucket from its first slot, then overflow |
| Alternate bucket hashing | `homeSlot = key % tableSize`; try the rest of that slot's bucket, then overflow |
| Rehashing | Larger prime table; every active record is hashed again with the new size |

Open-addressed slots are `EMPTY`, `OCCUPIED`, or `DELETED`. Searches continue past `DELETED` slots and stop at `EMPTY`.

## Menus

**Strategy menu:** 1 Linear Probing, 2 Quadratic Probing, 3 Double Hashing, 4 Separate Chaining, 5 Bucket Hashing, 6 Alternate Bucket Hashing + Overflow, 7 Compare All Techniques, 0 Exit.

**Application menu:**

1. Add Patient Record
2. Search for Patient
3. Update Patient Record
4. Delete Patient Record
5. Display Entire Hash Table
6. Display Patient Count
7. Display Hash Statistics
8. Generate Random Patient Records
9. Change Collision Strategy (existing records are moved into the new structure)
10. Rehash / Resize Table (manual rehash, or turn automatic rehash on/off)
11. Test Copy Constructor
12. Test Assignment Operator
13. Clear Database
14. Compare Hashing Techniques
0. Exit

## Class Design

```
DictionaryADT<K,V>                (abstract interface)
  HashTableBase<K,V>              (shared count, statistics, reports)
    OpenAddressHash<K,V>          (abstract: table of HashEntry<K,V>)
      LinearProbingHash<K,V>
      QuadraticProbingHash<K,V>
      DoubleHash<K,V>
    SeparateChainingHash<K,V>     (table of ChainNode<K,V>* chains)
    BucketHash<K,V>               (HashEntry slots + ChainNode overflow list)
      AlternateBucketHash<K,V>

PatientRecord, HashEntry<K,V>, ChainNode<K,V>, HashStatistics,
RandomPatientGenerator, MedicalHashApplication
```

The application holds a `DictionaryADT<int, PatientRecord>*`, so the menu code does not change when the strategy changes.

## Assumptions

- Patient ID is the key; manually entered IDs may be 1 to 999999 (small IDs make collisions easy to demonstrate). Random IDs are 100000 to 999999; duplicates are detected and regenerated.
- Table sizes are rounded up to a prime. Open-addressing tables have a minimum size of 5.
- Automatic rehash thresholds: linear and double hashing 0.7, quadratic probing 0.5, separate chaining 2.0. Bucket methods start with automatic rehash OFF (threshold 0.8) so overflow can be demonstrated; it can be turned on in menu 10.
- A rehash grows the table to the next prime at or above double the old size (and at least 2 x records + 1).
- Bucket hashing: number of buckets = next prime at or above (slots / bucket size). Default bucket size is 3.
- Statistics definitions:
  - **Probes**: positions or chain nodes examined to place a record (1 means no collision). Overflow insertion counts as one extra probe.
  - **Collision**: an insertion whose home position was already in use.
  - **Comparisons**: positions or nodes examined during a search.
  - Re-insertions performed during a rehash are not counted as new insertions.
- Records are inserted at the tail of a chain and of the overflow list.
- Compare All inserts the same dataset into every method, searches for every stored key (successful) and for the same number of absent keys (unsuccessful). It saves the dataset to `data/synthetic_patients.csv` and appends results to `data/results.csv`.

## Folder Structure

- `src/` - .cpp source files
- `include/` - .h header files (template classes are header-only)
- `docs/` - UML, poster, presentation, screenshots
- `data/` - synthetic datasets and comparison results
