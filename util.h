#ifndef _UTIL_H_
#define _UTIL_H_

#define _swap(a, b) ({\
	__typeof__(a) tmp = a;\
	a = b;\
	b = tmp;\
	(void)0;\
})

#define _min(a, b) ({\
    __typeof__(a) _a = a;\
    __typeof__(b) _b = b;\
    _a < _b ? _a : _b;\
})

#define _max(a, b) ({\
    __typeof__(a) _a = a;\
    __typeof__(b) _b = b;\
    _a > _b ? _a : _b;\
})

#endif /* _UTIL_H_ */