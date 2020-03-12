#ifndef __BASE_NONCOPYABLE_H__
#define __BASE_NONCOPYABLE_H__

namespace base
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
