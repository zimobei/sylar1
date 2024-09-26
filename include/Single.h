//*****************************************************************************
//
//
//   ��ͷ�ļ�ʵ�ֵ���ģʽ��ؽӿ�����ķ�װ
//  
//
//*****************************************************************************


#ifndef SYLAR_SINGLE_H
#define SYLAR_SINGLE_H

#include <memory>

namespace sylar {

template<class T>
class Single {
private:
	Single(){}
	~Single(){}

	Single(const Single& single) = delete;
	Single(Single&& single) = delete;
	const Single& operator=(const Single& single) = delete;
	const Single& operator=(Single&& single) = delete;
public:
	static std::shared_ptr<T> GetInstance() {
		static std::shared_ptr<T> result = std::make_shared<T>();
		return result;
	}
};

}; /* sylar */

#endif /* SYLAR_SINGLE_H */