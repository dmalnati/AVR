#ifndef __TYPE_TRAITS_H__
#define __TYPE_TRAITS_H__


// https://stackoverflow.com/questions/13071340/how-to-check-if-two-types-are-same-at-compiletimebonus-points-if-it-works-with


template<class T, class U>
struct is_same {
        enum { value = 0 };
};

template<class T>
struct is_same<T, T> {
        enum { value = 1 };
};


#endif  // __TYPE_TRAITS_H__