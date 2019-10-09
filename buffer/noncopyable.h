#ifndef COMMON_NONCOPYABLE_H
#define COMMON_NONCOPYABLE_H

namespace common
{

	class noncopyable
	{
 		public:
  			noncopyable(const noncopyable&) = delete;
  			void operator=(const noncopyable&) = delete;
	
 			protected:
  			noncopyable() = default;
  			~noncopyable() = default;
	};

}

#endif
