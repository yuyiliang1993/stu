#include<iostream>
#include<string>
#include<list>
#include<memory>

//观察者模式
//20190809


/*
note:
**充分利用指针地址，继承，多态
**观察者和和具体被观察得类采用继承多态使用利用list得push和remove来维护被观察得集合
**使用iterator 轮询每一个实例


Subject（目标）
——目标知道它的观察者。可以有任意多个观察者观察同一个目标；
——提供注册和删除观察者对象的接口。

Observer（观察者）
——为那些在目标发生改变时需获得通知的对象定义一个更新接口。

ConcreteSubject（具体目标）
——将有关状态存入各ConcreteObserver对象；
——当它的状态发生改变时，向它的各个观察者发出通知。

ConcreteObserver（具体观察者）
——维护一个指向ConcreteSubject对象的引用；
——存储有关状态，这些状态应与目标的状态保持一致；
——实现Observer的更新接口以使自身状态与目标的状态保持一致。

观察者模式按照以下方式进行协作：

当ConcreteSubject发生任何可能导致其观察者与其本身状态不一致的改变时，它将通知它的各个观察者；
在得到一个具体目标的改变通知后，ConcreteObserver对象可向目标对象查询信息。ConcreteObserver使用这些信息以使它的状态与目标对象的状态一致。
*/

class Observer{
public :
	virtual void update(const std::string & msg)=0;
	virtual const std::string & getname()=0;
};

class ConcreteObserver:public Observer{
public:
	ConcreteObserver()=default;
	ConcreteObserver(const std::string &name){
		m_name = name;
	 }
	 
	void update(const std::string & msg) override{
		std::cout<<"ConcreteObserver update:"<<msg<<std::endl;
		m_name = msg;
	}

	const std::string &getname()
	{
		return m_name;
	}
private:
	std::string m_name;

};


class Subject{
public:
	virtual void registerObser(Observer *ptr)=0;
	virtual void deleteObser(Observer*ptr)=0;
	virtual void notifyObser(const std::string msg)=0;
};


class ConcreteSubject:public Subject
{
public:
	ConcreteSubject(){
		l=new std::list<Observer *>();
	}
	~ConcreteSubject(){
		delete l;
	}
	
	void registerObser(Observer *ptr)override{
		l->push_back(ptr);
	}

	void deleteObser(Observer *ptr)override{
		l->remove(ptr);
	}
	void notifyObser(const std::string msg)override{
		std::list<Observer*>::iterator iter;
		for(iter=l->begin();iter!=l->end();++iter){
			(*iter)->update(msg);
		}
	}
	void show()
	{
		std::list<Observer*>::iterator iter;
		int c = 0;
		std::cout<<"list show:"<<std::endl;
		for(iter=l->begin();iter!=l->end();++iter){
			std::cout<<c<<":"<<(*iter)->getname()<<std::endl;
			c++;
		}
	}

private:
	std::list<Observer *> *l;
};


void test_observer()
{
	ConcreteObserver *r1 = new ConcreteObserver("qwe");
	ConcreteObserver *r2 = new ConcreteObserver("asd");
	ConcreteObserver *r3 = new ConcreteObserver("zxc");
	ConcreteSubject *r = new ConcreteSubject();
	r->registerObser(r1);
	r->registerObser(r2);
	r->registerObser(r3);
	r->show();
	r->notifyObser("qweasdzxc123");
	r->show();
	delete r1;
	delete r2;
	delete r3;
	delete r;
}

int main(int argc,char**argv)
{
	test_observer();
}
