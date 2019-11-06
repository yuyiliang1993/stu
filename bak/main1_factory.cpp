#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
template <typename T>
class A{
public:
	A()
	{
		ptr=std::make_shared<std::vector<T>>();
	}
	
	A(T v){
		ptr=std::make_shared<std::vector<T>>();
	}
	
	void show()
	{
		auto r= ptr;
		auto x= r;
		std::cout<<"ptr_count:"<<ptr.use_count()<<std::endl;
		for(int i = 0;i<ptr->size();++i){
			std::cout<<"val:"<<ptr->at(i)<<std::endl;
		}
	}

	void insert(T v)
	{
		ptr->push_back(v);
	}
	

private:
		T val;
		std::shared_ptr<std::vector<T>> ptr;
};



void test_shared_ptr()
{
	A<int> a;
	a.insert(100);
	a.insert(101);
	a.insert(102);
	a.show();
}


class tank
{
public:
	virtual void message() = 0;
};

class tank90:public tank
{
public:
	void message()override {
		std::cout<<"tank90"<<std::endl;
	}
};

class tank80:public tank
{
public:
	void message()override{
		std::cout<<"tank80"<<std::endl;
	}
};

enum TankType{
	tank80_type,
	tank90_type,
};

class factoryTank{
public:
		tank *createTank(TankType t){
			switch(t){
				case tank80_type:
					return new tank80();
				case tank90_type:
					return new tank90();
				default:
				return nullptr;
			}
		}
};


void test_factory_mode(){
	factoryTank *f=new factoryTank();
	tank *p=f->createTank(tank90_type);
	if(p){
		p->message();
	}
	delete f;
}

std::mutex mt;
int main()
{
#if 0
	test_shared_ptr();

#endif
	mt.lock();
	test_factory_mode();
	mt.unlock();
}
