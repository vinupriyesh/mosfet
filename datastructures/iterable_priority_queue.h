#ifndef ITERABLE_PRIORITY_QUEUE_H
#define ITERABLE_PRIORITY_QUEUE_H

#include <queue>
#include <vector>

template <typename T>
/**
 * A priority queue that is iterable
 * 
 * usage: 
   int main() {
    IterablePriorityQueue<int> pq;
    pq.push(10);
    pq.push(30);
    pq.push(20);
    pq.push(5);
    pq.push(1);

    std::cout << "Elements in the priority queue:" << std::endl;
    for (const int& value : pq) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    return 0;
}
 */
class IterablePriorityQueue : public std::priority_queue<T, std::vector<T>, std::greater<T>> {
public:
    using std::priority_queue<T, std::vector<T>, std::greater<T>>::c; // Expose the underlying container
    typename std::vector<T>::const_iterator begin() const { return this->c.cbegin(); }
    typename std::vector<T>::const_iterator end() const { return this->c.cend(); }
};

#endif //ITERABLE_PRIORITY_QUEUE_H