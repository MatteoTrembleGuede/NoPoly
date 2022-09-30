/// <summary> call like this : delegate += new TDef::OMC<T>( T* objPtr, T::* FuncPtr ); </summary>
template<typename... B>
template<class T>
void Delegate<B...>::operator+=(ObjectMethodPair<T>* ref)
{
	pointers.push_back(ref);
}

/// <summary> call like this : delegate -= new TDef::OMC<T>( T* objPtr, T::* FuncPtr ); </summary>
template<typename... B>
template<class T>
void Delegate<B...>::operator-=(ObjectMethodPair<T>* ref)
{
	pointers.remove_if([&](FunctionPointerObject* c) {
		ObjectMethodPair<T>* ct = dynamic_cast<ObjectMethodPair<T>*>(c);
		if (ct && ct->object == ref->object && ct->func == ref->func)
		{
			delete c;
			return true;
		}
		else
		{
			return false;
		}
		});
	delete ref;
}

template<typename... B>
template<class T>
void Delegate<B...>::ObjectMethodPair<T>::operator()(B... args)
{
	(object->*func)(args...);
}

template<typename... B>
void Delegate<B...>::SimpleFPO::operator()(B... args)
{
	(*func)(args...);
}

template<typename... B>
void Delegate<B...>::operator()(B... args)
{
	for (auto it = pointers.begin(); it != pointers.end(); ++it)(**it)(args...);
}

template<typename... B>
template<class T>
void Delegate<B...>::Bind(T* _object, typename ObjectMethodPair<T>::FuncPtr _func)
{
	*this += new ObjectMethodPair<T>(_object, _func);
}

template<typename... B>
template<class T>
void Delegate<B...>::Unbind(T* _object, typename ObjectMethodPair<T>::FuncPtr _func)
{
	*this -= new ObjectMethodPair<T>(_object, _func);
}


template<typename... B>
void Delegate<B...>::operator+=(void (*Ptr) (B... args))
{
	pointers.push_back(new SimpleFPO(Ptr));
}

template<typename... B>
void Delegate<B...>::operator-=(void (*Ptr) (B... args))
{
	pointers.remove_if([&](FunctionPointerObject* c) {
		SimpleFPO* ct = dynamic_cast<SimpleFPO*>(c);
		if (ct && ct->func == Ptr)
		{
			delete c;
			return true;
		}
		else
		{
			return false;
		}
		});
}

template<typename... B>
void Delegate<B...>::Bind(void (*Ptr) (B... args))
{
	*this += Ptr;
}

template<typename... B>
void Delegate<B...>::Unbind(void (*Ptr) (B... args))
{
	*this -= Ptr;
}