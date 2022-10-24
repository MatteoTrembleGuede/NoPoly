#pragma once
#include <list>
#include <vector>
#include <string>
#include <functional>
#include <stdexcept>

#define WINPOPUP // Comment this if you don't have WinPopup.h

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
	public:
		bool isKilled = false;
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

	struct LambdaFPO : public FunctionPointerObject
	{
		using FuncPtr = std::function<void(B... args)>;
		FuncPtr func;
		LambdaFPO(FuncPtr _func) : func(std::move(_func)) {}
		void operator()(B... args) override;
	};
	
	struct DelegateFPO : public FunctionPointerObject
	{
		typedef Delegate<B...> FuncPtr;
		FuncPtr& func;
		DelegateFPO(FuncPtr& _func) : func(_func) {}
		void operator()(B... args) override;
	};

	~Delegate() 
	{
		Clear(); 
		isKilled = true;
	}

private:

	bool isExecuting = false;
	bool isKilled = false;
	bool mustClear = false;
	std::list<FunctionPointerObject*> pointers;
	std::vector<FunctionPointerObject*> safeRemoveList;

public:


	template<class T>
	void SafeRemove(T* _object, typename ObjectMethodPair<T>::FuncPtr _func);
	void SafeRemove(void (*Ptr) (B... args));
	void SafeRemove(Delegate<B...>& del);

	void Clear();
	bool ContainsDelegate(Delegate<B...>& del);

	// Bind object with method

	/// call like this : delegate -= new TDef::ObjectMethodPair<T>( T* objPtr, T::* FuncPtr );
	template<class T>
	void operator-=(ObjectMethodPair<T>* ref);

	/// call like this : delegate += new TDef::ObjectMethodPair<T>( T* objPtr, T::* FuncPtr );
	template<class T>
	void operator+=(ObjectMethodPair<T>* ref);

	template<class T>
	void Bind(T* _object, typename ObjectMethodPair<T>::FuncPtr _func);
	
	template<class T>
	void Unbind(T* _object, typename ObjectMethodPair<T>::FuncPtr _func);
	
	
	// Bind function

	void operator-=(void (*Ptr) (B... args));

	// be carefull as capturing lambdas won't unbind unless you clear the delegate
	// 
	// if you assign a lambda (non capturing) ensure you stored it (ex : auto l = +[]{}; del += l; del -= l;) unless you won't be able to unbind it 
	// 
	// non capturing lambdas cause ambiguous call, try +[] instead of [], this will cause the compiler to choose the overload with parameter of type void (*) (B... args)
	void operator+=(void (*Ptr) (B... args));

	// be carefull as capturing lambdas won't unbind unless you clear the delegate
	// 
	// if you assign a lambda (non capturing) ensure you stored it (ex : auto l = +[]{}; del += l; del -= l;) unless you won't be able to unbind it 
	// 
	// non capturing lambdas cause ambiguous call, try +[] instead of [], this will cause the compiler to choose the overload with parameter of type void (*) (B... args)
	void Bind(void (*Ptr) (B... args));

	void Unbind(void (*Ptr) (B... args));


	// Bind Delegate

	void operator-=(Delegate<B...>& del);

	void operator+=(Delegate<B...>& del);

	void Bind(Delegate<B...>& del);

	void Unbind(Delegate<B...>& del);


	// Bind lambda

	// be carefull as capturing lambdas won't unbind unless you clear the delegate
	// 
	// if you assign a lambda (non capturing) ensure you stored it (ex : auto l = +[]{}; del += l; del -= l;) unless you won't be able to unbind it 
	// 
	// non capturing lambdas cause ambiguous call, try +[] instead of [], this will cause the compiler to choose the overload with parameter of type void (*) (B... args)
	void operator+=(std::function<void(B...)> lambda);

	// be carefull as capturing lambdas won't unbind unless you clear the delegate
	// 
	// if you assign a lambda (non capturing) ensure you stored it (ex : auto l = +[]{}; del += l; del -= l;) unless you won't be able to unbind it 
	// 
	// non capturing lambdas cause ambiguous call, try +[] instead of [], this will cause the compiler to choose the overload with parameter of type void (*) (B... args)
	void Bind(std::function<void(B...)> lambda);


	void operator()(B... args);
};

DeclareDelegate(Notify); // simple on action call of functions with no parameters and no return value

#include "Delegate.inl"