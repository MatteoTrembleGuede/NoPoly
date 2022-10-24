#ifdef WINPOPUP
#include "WinPopup.h"
#endif

template<typename... B>
void Delegate<B...>::Clear()
{
	if (isExecuting) mustClear = true;
	else
	{
		std::vector<FunctionPointerObject*> tmp;
		if (pointers.size() > 0)
			for (auto it : pointers)
				tmp.push_back(it);

		pointers.clear();
		safeRemoveList.clear();

		for (int i = 0; i < tmp.size(); ++i) delete tmp[i];

		mustClear = false;
	}
}

template<typename... B>
template<class T>
void Delegate<B...>::operator+=(ObjectMethodPair<T>* ref)
{
	pointers.push_back(ref);
}

template<typename... B>
template<class T>
void Delegate<B...>::operator-=(ObjectMethodPair<T>* ref)
{
	if (isExecuting)
	{
		SafeRemove(ref->object, ref->func);
	}
	else
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
	}
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
void Delegate<B...>::LambdaFPO::operator()(B... args)
{
	func(args...);
}

template<typename... B>
void Delegate<B...>::DelegateFPO::operator()(B... args)
{
	func(args...);
}

template<typename... B>
void Delegate<B...>::operator()(B... args)
{
	if (isExecuting == true)
	{
#ifdef WINPOPUP
		WinPopup("delegate is in a loop executing itself", "error", MessageType::Error);
#endif
		throw std::overflow_error("delegate is in a loop executing itself");
	}

	int size = pointers.size();
	isExecuting = true;
	for (auto it = pointers.begin(); it != pointers.end(); ++it)
	{
		if (!(*it)->isKilled)
		{
			(**it)(args...);
			if (isKilled)
			{
#ifdef WINPOPUP
					WinPopup("delegate was deleted during execution", "Error", MessageType::Error);
#endif
					throw std::runtime_error("delegate was deleted during execution");
			}
		}
	}
	isExecuting = false;

#ifdef _DEBUG
	if (size < pointers.size())
	{
#ifdef WINPOPUP
		std::string message = "one or more of the contained functions is/are assigning to the container delegate : \nsize increased from ";
		message += std::to_string(size);
		message += " to ";
		message += std::to_string(pointers.size());
		message += " during execution";
		WinPopup(message.c_str(), "warning", MessageType::Warning);
#endif
	}
#endif // _DEBUG

	if (mustClear) Clear();

	for (FunctionPointerObject*& func : safeRemoveList)
	{
		pointers.remove(func);
		delete func;
	}
	safeRemoveList.clear();
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
	if (isExecuting)
	{
		SafeRemove(Ptr);
	}
	else
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
template<typename... B>
void Delegate<B...>::operator+=(Delegate<B...>& del)
{
	if (&del == this)
	{
#ifdef WINPOPUP
		WinPopup("delegate was assigned to itself", "error", MessageType::Error);
#endif
		throw std::invalid_argument("delegate was assigned to itself");
	}
	else if (del.ContainsDelegate(*this))
	{
#ifdef WINPOPUP
		WinPopup("a delegate contains the delegate it was assigned to", "error", MessageType::Error);
#endif
		throw std::invalid_argument("a delegate contains the delegate it was assigned to");
	}
	pointers.push_back(new DelegateFPO(del));
}

template<typename... B>
bool Delegate<B...>::ContainsDelegate(Delegate<B...>& del)
{
	for (FunctionPointerObject*& obj : pointers)
	{
		DelegateFPO* objP = dynamic_cast<DelegateFPO*>(obj);
		if (objP)
		{
			if (&objP->func == &del)
			{
				return true;
			}
			else if (objP->func.ContainsDelegate(del)) return true;
		}
	}
	return false;
}

template<typename... B>
void Delegate<B...>::operator-=(Delegate<B...>& del)
{
	if (isExecuting)
	{
		SafeRemove(del);
	}
	else
	{
		pointers.remove_if([&](FunctionPointerObject* c) {
			DelegateFPO* ct = dynamic_cast<DelegateFPO*>(c);
			if (ct && &ct->func == &del)
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
}

template<typename... B>
void Delegate<B...>::Bind(Delegate<B...>& del)
{
	*this += del;
}

template<typename... B>
void Delegate<B...>::Unbind(Delegate<B...>& del)
{
	*this -= del;
}

template<typename... B>
void Delegate<B...>::operator+=(std::function<void(B...)> lambda)
{
	pointers.push_back(new LambdaFPO(lambda));
}

template<typename... B>
void Delegate<B...>::Bind(std::function<void(B...)> lambda)
{
	*this += lambda;
}

template<typename... B>
template<class T>
void Delegate<B...>::SafeRemove(T* _object, typename ObjectMethodPair<T>::FuncPtr _func)
{
	for (FunctionPointerObject*& func : pointers)
	{
		ObjectMethodPair<T>* fpo = dynamic_cast<ObjectMethodPair<T>*>(func);
		if (fpo == nullptr) continue;
		if (fpo->object == _object && fpo->func == _func)
		{
			fpo->isKilled = true;
			safeRemoveList.push_back(fpo);
			return;
		}
	}
}

template<typename... B>
void Delegate<B...>::SafeRemove(void (*Ptr) (B... args))
{
	for (FunctionPointerObject*& func : pointers)
	{
		SimpleFPO* fpo = dynamic_cast<SimpleFPO*>(func);
		if (fpo == nullptr) continue;
		if (fpo->func == Ptr)
		{
			fpo->isKilled = true;
			safeRemoveList.push_back(fpo);
			return;
		}
	}
}

template<typename... B>
void Delegate<B...>::SafeRemove(Delegate<B...>& del)
{
	for (FunctionPointerObject*& func : pointers)
	{
		DelegateFPO* fpo = dynamic_cast<DelegateFPO*>(func);
		if (fpo == nullptr) continue;
		if (&fpo->func == &del)
		{
			fpo->isKilled = true;
			safeRemoveList.push_back(fpo);
			return;
		}
	}
}