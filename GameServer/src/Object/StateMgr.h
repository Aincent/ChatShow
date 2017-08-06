/*
 * StateMgr.h
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */

#ifndef STATEMGR_H_
#define STATEMGR_H_

#pragma once
//-----------------------------------------------------------------------------
// 类声明
//-----------------------------------------------------------------------------
template<class T, typename E>
class State
{
public:
	State();

	void	InitState(T tState);

	bool	IsInState(E eState) const;
	bool	IsInStateAll(T tState) const;
	bool	IsInStateAny(T tState) const;

	T		GetState() const;
	void	SetState(E eState);
	void	UnsetState(E eState);

private:
	T		m_tState;
};


//-----------------------------------------------------------------------------
// 内联实现
//-----------------------------------------------------------------------------
template<class T, typename E>
inline State<T, E>::State()
{
	memset(this, 0, sizeof(*this));
}

template<class T, typename E>
inline void State<T, E>::InitState(T tState)
{
	m_tState = tState;
}

template<class T, typename E>
inline T State<T, E>::GetState() const
{
	return m_tState;
}

template<class T, typename E>
inline bool State<T, E>::IsInState(E eState) const
{
	return (m_tState & (T)eState);
}

template<class T, typename E>
inline bool State<T, E>::IsInStateAll(T tState) const
{
	return (m_tState & tState) == tState;
}

template<class T, typename E>
inline bool State<T, E>::IsInStateAny(T tState) const
{
	return m_tState & tState;
}

template<class T, typename E>
inline void State<T, E>::SetState(E eState)
{
	m_tState |= eState;
}

template<class T, typename E>
inline void State<T, E>::UnsetState(E eState)
{
	if(IsInState(eState))
	{
		m_tState ^= eState;
	}
}

#endif /* STATEMGR_H_ */
