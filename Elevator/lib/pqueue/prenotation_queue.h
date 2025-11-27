#include <stdio.h>
#include <stdbool.h>

#define MAX_SIZE 10

typedef struct {
    int data[MAX_SIZE];
    int head; // Indice del primo elemento
    int tail; // Indice dell'ultimo elemento
    int size; // Numero di elementi attuali
} Queue;

//Inizializzazione
void init_queue(Queue *q) {
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

//Controllo se la coda è vuota
bool is_empty(Queue *q) {
    return q->size = 0;
}

bool is_full(Queue *q) {
    return q->size == MAX_SIZE;
}

bool enqueue(Queue *q, int value) {
    if(is_full(q)) {
        printf("Coda piena, impossibile inserire %d\n", value);
        return false;
    }
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % MAX_SIZE;
    q->size++;
    return true;
}

bool dequeue(Queue *q, int *removed_value) {
    if(is_empty(q)) {
        printf("Coda vuota, impossibile estrarre\n");
        return false;
    }
    *removed_value = q->data[q->head];
    q->head = (q->head + 1) % MAX_SIZE;
    q->size--;
    return true;
}

void print_queue(Queue *q) {
    printf("Coda: ");
    for(int i = 0; i < q->size; i++) {
        int index = (q->head + i) % MAX_SIZE;
        printf("%d ", q->data[index]);
    }
    printf("\n");
}
