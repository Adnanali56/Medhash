#include <iostream>
using namespace std;

int main()
{
    int choice = -1;
    while (choice != 0)
    {
        cout << "==================================================\n";
        cout << "   MEDICAL RECORD HASHING SYSTEM\n";
        cout << "==================================================\n\n";
        cout << "Select Hashing / Collision Strategy\n\n";
        cout << "1. Linear Probing\n";
        cout << "2. Quadratic Probing\n";
        cout << "3. Double Hashing\n";
        cout << "4. Separate Chaining\n";
        cout << "5. Bucket Hashing\n";
        cout << "6. Alternate Bucket Hashing + Overflow\n";
        cout << "7. Compare All Techniques\n";
        cout << "0. Exit\n\n";
        cout << "Selection: ";
        cin >> choice;

        if (choice >= 1 && choice <= 7)
            cout << "\n(Not implemented yet)\n\n";
        else if (choice != 0)
            cout << "\nInvalid choice.\n\n";
    }
    cout << "Goodbye.\n";
    return 0;
}
