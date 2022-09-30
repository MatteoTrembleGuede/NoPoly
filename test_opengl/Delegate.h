#pragma once
#include <list>
#include <vector>

/// <summary> used to declare any delegate </summary>
/// <param name="TDef">Name of the delegate type</param>
/// <param name="...">types of the parameters of the delegate</param>
/// <returns>void</returns>
#define DeclareDelegate(TDef, ...)	typedef Delegate<##__VA_ARGS__> TDef; 

template<typename... B>
class Delegate
{
public:

	struct FunctionPointerObject
	{
		virtual void operator()(B... args) = 0;
	};

	template<class T>
	struct ObjectMethodPair : public FunctionPointerObject
	{
		typedef void (T::* FuncPtr) (B... args);
		T* object;
		FuncPtr func;
		ObjectMethodPair(T* _object, FuncPtr _func) : object(_object), func(_func) {}
		void operator()(B... args) override;
	};
	
	struct SimpleFPO : public FunctionPointerObject
	{
		typedef void (*FuncPtr) (B... args);
		FuncPtr func;
		SimpleFPO(FuncPtr _func) : func(_func) {}
		void operator()(B... args) override;
	};

	void Clear()
	{
		std::vector<FunctionPointerObject*> tmp;
		if (pointers.size() > 0)
		for (auto it : pointers)
			tmp.push_back(it);

		pointers.clear();

		for (int i = 0; i < tmp.size(); ++i) delete tmp[i];
	}

	~Delegate() 
	{ 
		Clear();
	}

private:

	std::list<FunctionPointerObject*> pointers;

public:

	// Bind object with method

	/// <summary> call like this : delegate -= new TDef::OMC<T>( T* objPtr, T::* FuncPtr ); </summary>
	template<class T>
	void operator-=(ObjectMethodPair<T>* ref);

	/// <summary> call like this : delegate += new TDef::OMC<T>( T* objPtr, T::* FuncPtr ); </summary>
	template<class T>
	void operator+=(ObjectMethodPair<T>* ref);

	template<class T>
	void Bind(T* _object, typename ObjectMethodPair<T>::FuncPtr _func);
	
	template<class T>
	void Unbind(T* _object, typename ObjectMethodPair<T>::FuncPtr _func);
	
	
	// Bind function
	
	void operator-=(void (*Ptr) (B... args));

	void operator+=(void (*Ptr) (B... args));

	void Bind(void (*Ptr) (B... args));
	
	void Unbind(void (*Ptr) (B... args));

	void operator()(B... args);
};

DeclareDelegate(Notify); // simple on action call of functions with no parameters and no return value

#include "Delegate.inl"

