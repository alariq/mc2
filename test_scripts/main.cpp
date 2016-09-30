

template<typename T>
class List {
    public:

        List():data(new T[16]) {}

    T* data;


    class Iterator {
        public:
            explicit Iterator(T* v):iter(v) {}
            //explicit Iterator(const Iterator& v):iter(v.iter) {}
        T* iter;
    };

    class ConstIterator: public Iterator {
        public:
            explicit ConstIterator(T* v):Iterator(v) {}

            T* get() { return Iterator::iter; };

    };

    Iterator begin() { return Iterator(data); }
    ConstIterator begin() const { return ConstIterator(data); }

} ;

int main(void)
{
    List<int> my_list;
    List<int>::Iterator iter = my_list.begin();

    const List<int> my_const_list;
    List<int>::ConstIterator citer = my_const_list.begin();
    int* v = citer.get();

}
