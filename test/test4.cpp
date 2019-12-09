#include<iostream>

//"qwerty"
class MyclassA{
public :
	
	MyclassA(int a){
		std::cout<<"MyclassA(int a)"<<std::endl;
		value = a;
	}

	virtual void print(){
		std::cout<<"I am MyclassA"<<std::endl;
	}

	virtual void fun() const{
		std::cout<<"I am MyclassA fun"<<std::endl;
	}

	
	int value;
};


class MyclassB:public MyclassA{
public:
	MyclassB():x(1),y(100),MyclassA(100){
	}
	virtual void print() override{
		std::cout<<"I am MyclassB"<<std::endl;
	}
	virtual void fun()const{
		std::cout<<"I am MyclassB fun"<<std::endl;
	}
	
private:
	int x;
	int y;
}; 

int main(){
	const MyclassA *p = new MyclassB;
	p->print();
	
	MyclassA a(100);
	a.fun();
		
//	MyclassA a;
//	std::cout<<"value:"<<a.value<<std::endl;
}
