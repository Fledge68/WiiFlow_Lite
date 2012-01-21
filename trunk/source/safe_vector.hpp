/*****************************************|
|-----vector class wrapper by Miigotu-----|
|---Thx to dimok and r-win for guidance---|
|*****************************************/

#ifndef SAFE_VECTOR
#define SAFE_VECTOR

#include <string>
#include <vector>

template <class T>
class safe_vector
{
    public:
		typedef size_t size_type;
		typedef typename std::vector<T>::iterator iterator;
		typedef typename std::vector<T>::const_iterator const_iterator;
		typedef typename std::vector<T>::reference reference;
		typedef typename std::vector<T>::const_reference const_reference;

        safe_vector(){};
        safe_vector(size_type n){thevector.resize(n);}
        ~safe_vector(){clear();};

        void clear()
		{
			thevector.clear();
			std::vector<T>().swap(thevector);
		}

		void push_back(const T& x)
		{
			if(thevector.size() * sizeof(T) == thevector.capacity() && thevector.capacity() < thevector.max_size() - 20)
				thevector.reserve(thevector.size() + 20);
			thevector.push_back(x);
		}

		void resize(size_type sz, T c = T())
		{
			thevector.resize(sz, c);
			realloc(sz);
		}

		size_type size() const { return thevector.size(); }
		
		void reserve(size_type n) {thevector.reserve(n);}

		size_type capacity() const {return thevector.capacity();}

		bool empty() const {return thevector.empty();}

		reference operator[](size_type n) {return thevector[n];}
		const_reference operator[](size_type n) const {return thevector[n];}
		
		iterator erase(iterator position) {return thevector.erase(position);}
		iterator erase(iterator first, iterator last) {return thevector.erase(first, last);}
		
		iterator begin() {return thevector.begin();}
		const_iterator begin() const {return thevector.begin();}

		iterator end() {return thevector.end();}
		const_iterator end() const {return thevector.end();}
		
		const_reference at (size_type n) const {return thevector.at(n);}
		reference at (size_type n) {return thevector.at(n);}

		reference back() {return thevector.back();}
		const_reference back() const {return thevector.back();}
		
		void realloc(size_type sz)
		{
			if(thevector.size() * sizeof(T) < thevector.capacity() || sz * sizeof(T) < thevector.capacity() || sz < thevector.size())
			{
				iterator itr;

				std::vector<T> newvector;
				newvector.reserve(sz);
				for (itr = thevector.begin();  newvector.size() < sz && itr < thevector.end(); itr++)
					newvector.push_back(*itr);

				clear();

				thevector.reserve(sz);
				for (itr = newvector.begin(); thevector.size() < sz && itr < newvector.end(); itr++)
					thevector.push_back(*itr);

				newvector.clear();
				std::vector<T>().swap(newvector);
			}
		}
	private:
        std::vector<T> thevector;
};

#endif /*- SAFE_VECTOR -*/