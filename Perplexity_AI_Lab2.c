/*
EECS 348 Assignment 2 - CEO Email Priority Queue

Description: 
Implements a listbased MaxHeap to prioritize
a busy CEO's incoming emails. Emails are ranked first by sender category
(Boss > Subordinate > Peer > ImportantPerson > OtherPerson); ties within
the same category are broken by date, with the newest email read first.
Supports EMAIL, NEXT, READ, and COUNT commands read line-by-line from a test file.
 
Input:
A single command-line argument: the path to a text file containing
EMAIL / NEXT / READ / COUNT commands, one per line, in the format
specified in the Assignment 2 instructions.

Output:
Terminal output for the NEXT and COUNT commands, formatted exactly to
the assignment specification.

Collaborators: None

Other Sources: Perplexity Generative AI (as required by assignment)

Author: Ismail

Creation Date: 17 September, 2026
Revision Date: 17 September, 2026

Revisions: Added prolouge and inline comments. 


*/
#include <stdio.h>  //standard library header
#include <stdlib.h>  //library
#include <string.h>  //library

#define MAX_EMAILS 1000 // Defines the maximum number of emails the heap can hold
#define MAX_STR 256 // Defines the maximum string length for subje

typedef struct {
    char sender[64];    // Stores the sender category (e.g., Boss, Peer)
    char subject[256];  // Stores the subject line of the email
    char date[16];      // Stores the date string in MM-DD-YYYY format
    int priority;       // Sender priority integer
    int dateKey;        // Tie breaker date integer
} Email;

Email heap[MAX_EMAILS];  //array for the maxheap
int heapSize = 0;      //number of emails in the queue

int getCategoryPriority(char* category) {  //function converts sender into a numerical priority order
    if (strcmp(category, "Boss") == 0) return 5;  //boss has highest priority of 5
    if (strcmp(category, "Subordinate") == 0) return 4;  //Subordinate has priority of 4
    if (strcmp(category, "Peer") == 0) return 3;  //Peer has 3 priority
    if (strcmp(category, "ImportantPerson") == 0) return 2;  //ImportantPerson has priority of 2
    if (strcmp(category, "OtherPerson") == 0) return 1; //This is read last
    return 0;    //fallback for unrecognized categories
}

int parseDateKey(char* date) {       //parses MM-DD-YY into a sortable integer (YYYYMMDD)
    int month, day, year;  //month, day, year are integers
    sscanf(date, "%d-%d-%d", &month, &day, &year); // Extracts digits from the date string
    return year * 10000 + month * 100 + day; //structures the date so newer dates are larger integers
}

int compare(Email a, Email b) { //compares two email for order
    if (a.priority != b.priority) return a.priority - b.priority; // Compares primary priority first
    return a.dateKey - b.dateKey; // Breaks ties by returning the newer date key
}

void swap(Email* a, Email* b) { // Swaps two emails in the heap array
    Email temp = *a; // Stores first email in a temporary variable
    *a = *b;         // Overwrites first email with second email
    *b = temp;       // Places temporary email into the second slot
}

void heapifyUp(int index) { //bubbles elements up
    while (index > 0) { //continues until root is reached
        int parent = (index - 1) / 2; // Calculates the parent index in the list-based heap
        if (compare(heap[index], heap[parent]) > 0) { // If the current node is greater than its parent
            swap(&heap[index], &heap[parent]); // Swap them to maintain MaxHeap property
            index = parent;  // Update index to continue checking upwards
        } else {
            break; // Stop if the heap property is satisfied
        }
    }
}

void heapifyDown(int index) { //Restores heap structure by sinking element down after root is removed
    while (1) { // Infinite loop broken when heap property is satisfied
        int left = 2 * index + 1;  // Calculates left child index
        int right = 2 * index + 2; // Calculates right child index
        int largest = index;       // Assumes current index is the largest initially
        if (left < heapSize && compare(heap[left], heap[largest]) > 0) // Checks if left child exists and is greater than current largest
            largest = left; //assign current largest to left
        if (right < heapSize && compare(heap[right], heap[largest]) > 0) // Checks if right child exists and is greater than current largest
            largest = right; //assign current largest to right
        if (largest != index) { // If the largest is not the current index
            swap(&heap[index], &heap[largest]); //swap current with largest
            index = largest; // Update index to continue checking downwards
        } else {
            break; // Stop if the heap property is satisfied
        }
    }
}

void insertEmail(Email e) { //inserts an email into priority queue
    if (heapSize >= MAX_EMAILS) return; // Prevents overflow if queue is full
    heap[heapSize] = e;                 // Places new email at the end of the heap list
    heapifyUp(heapSize);                // Bubbles the new email up to its correct priority position
    heapSize++;                         // Increments the total email count
}

Email* peekEmail() { // Returns a pointer to the highest priority email without removing it
    if (heapSize == 0) return NULL; // Returns NULL if there are no emails to read
    return &heap[0];                // Returns the root of the MaxHeap
}

Email removeEmail() { // Removes and returns the highest priority email from the queue
    Email empty = {"", "", "", 0, 0}; // Creates an empty dummy email
    if (heapSize == 0) return empty;  // Returns empty email if queue is empty
    Email top = heap[0];              // Stores the highest priority email (root)
    heap[0] = heap[heapSize - 1];     // Moves the last leaf node to the root position
    heapSize--;                       // Decreases the total email count
    if (heapSize > 0) heapifyDown(0); // Sinks the new root down to restore MaxHeap property
    return top;                       // Returns the original highest priority email
}

void trim(char* str) {  //trims extra space before and after lines
    int len = strlen(str); //length of string
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r' || str[len-1] == ' ')) { //loops back to trim excess lines
        str[--len] = '\0';  //replaces trailing characters with null terminators
    }
    int start = 0;  //start index
    while (str[start] == ' ') start++; //loops forward to find non space character
    if (start > 0) { //if leading space exists
        memmove(str, str + start, len - start + 1); //leftshift string to remove leading string
    }
}

int main(int argc, char* argv[]) { //main function
    FILE* file = fopen(argv[1], "r");  //open file
    if (!file) { //if file is not opened successfully
        printf("Could not open file.\n"); //print failure
        return 1; //exit with error
    }

    char line[512]; //store each line read from file
    while (fgets(line, sizeof(line), file)) { //gets file line by line until end of email
        trim(line);  //trim the file
        if (strlen(line) == 0) continue; //if the line is empty, skip it

        if (strncmp(line, "EMAIL", 5) == 0) {
            char* rest = line + 6;           // Skips the "EMAIL " prefix
            char* comma1 = strchr(rest, ','); // Finds the first comma separating sender and subject
            if (!comma1) continue;           // Skips if format is incorrect
            *comma1 = '\0';                  // Sender string ends here
            char* category = rest;           // Assigns pointer to the parsed sender category
            char* rest2 = comma1 + 1;         // Moves pointer to the start of the subject line
            char* comma2 = strrchr(rest2, ','); // Finds the second comma separating subject and date
            if (!comma2) continue;            // Skips if format is incorrect
            *comma2 = '\0';                   // Sender string ends here
            char* subject = rest2;            // Assigns pointer to the subject
            char* date = comma2 + 1;          // Assigns pointer to the date

            
            trim(category);//Trims empty space around category
            trim(subject);//trims empty space around subject
            trim(date); //Trims empty space around date

            Email e; // Creates a new Email struct instance
            strcpy(e.sender, category); //copies category into struct
            strcpy(e.subject, subject);//copies subject into struct
            strcpy(e.date, date);     //copies date into struct
            e.priority = getCategoryPriority(category); // Calculates and sets integer priority
            e.dateKey = parseDateKey(date);             // Calculates and sets integer date key
            insertEmail(e);                             // Inserts the fully formed email into the priority queue
        } else if (strcmp(line, "NEXT") == 0) { //next read command
            Email* e = peekEmail(); //looks at highest priority email
            if (e) { //if an email exists
                printf("Next email:\n");            // Prints required header
                printf("Sender: %s\n", e->sender);  // Prints sender category
                printf("Subject: %s\n", e->subject);// Prints subject line
                printf("Date: %s\n", e->date);      // Prints date
            } else {
                printf("No emails to read.\n");    // Handles edge case of empty queue
            }
        } else if (strcmp(line, "READ") == 0) {   //processes a read command
            removeEmail(); // Removes highest priority email from the heap
        } else if (strcmp(line, "COUNT") == 0) { //processes count command
            printf("There are %d emails to read.\n", heapSize); // Prints current unread queue size
        }
    }

    fclose(file); //close file
    return 0;  //indicates successful completion of program
}