use std::fmt::{Display, Formatter};
use std::rc::Rc;

use im::Vector;

use crate::{TesslaDisplay, TesslaInt, TesslaValue};
use crate::TesslaValue::*;

pub type TesslaList<T> = TesslaValue<Vector<T>>;

impl<T: Display + Clone> TesslaDisplay for Vector<T> {
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_str("List(")?;
        let mut start = true;
        for item in self.iter() {
            if start {
                start = false;
            } else {
                f.write_str(", ")?;
            }
            write!(f, "{}", item)?;
        }
        f.write_str(")")
    }
}

impl<T: Clone> TesslaList<T> {
    #[inline]
    pub fn append(&self, elem: T) -> TesslaList<T> {
        match self {
            Error(error) => Error(error),
            Value(list) => {
                let mut result = list.clone();
                result.push_back(elem);
                Value(result)
            }
        }
    }

    #[inline]
    pub fn empty() -> TesslaList<T> {
        Value(Vector::new())
    }

    #[inline]
    pub fn fold<A>(&self, start: TesslaValue<A>, function: TesslaValue<Rc<dyn Fn(TesslaValue<A>, T) -> TesslaValue<A>>>) -> TesslaValue<A> {
        match (self, function) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(list), Value(function)) => {
                let mut result = start;
                for item in list {
                    result = function(result, item.clone());
                }
                return result;
            }
        }
    }

    #[inline]
    pub fn init(&self) -> TesslaList<T> {
        match self {
            Error(error) => Error(error),
            Value(value) => {
                if value.len() > 1 {
                    Value(value.clone().slice(..(value.len() - 1)))
                } else {
                    Self::empty()
                }
            }
        }
    }

    #[inline]
    pub fn prepend(&self, elem: T) -> TesslaList<T> {
        match self {
            Error(error) => Error(error),
            Value(list) => {
                let mut result = list.clone();
                result.push_front(elem);
                return Value(result);
            }
        }
    }

    #[inline]
    pub fn set(&self, index: TesslaInt, elem: T) -> TesslaList<T> {
        match (self, index) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(list), Value(index)) => {
                let mut result = list.clone();
                result.set(index as usize, elem);
                Value(result)
            }
        }
    }

    #[inline]
    pub fn size(&self) -> TesslaInt {
        match self {
            Error(error) => Error(error),
            Value(value) => match i64::try_from(value.len()) {
                Ok(size) => Value(size),
                Err(_) => Error("Failed to convert usize to i64")
            },
        }
    }

    #[inline]
    pub fn tail(&self) -> TesslaList<T> {
        match self {
            Error(error) => Error(error),
            Value(value) => {
                if value.len() > 1 {
                    Value((value.clone()).split_off(1))
                } else {
                    Self::empty()
                }
            }
        }
    }

    #[inline]
    pub fn map<U: Clone>(&self, function: TesslaValue<Rc<dyn Fn(T) -> U>>) -> TesslaList<U> {
        match (self, function) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(list), Value(function)) => {
                let mut result = Vector::new();
                for item in list {
                    result.push_back(function(item.clone()));
                }
                Value(result)
            }
        }
    }
}

impl<T: Clone> TesslaList<TesslaValue<T>> {
    #[inline]
    pub fn get(&self, index: TesslaInt) -> TesslaValue<T> {
        match (self, index) {
            (Error(error), _) => Error(error),
            (_, Error(error)) => Error(error),
            (Value(list), Value(index)) => match list.get(index as usize) {
                Some(value) => value.clone(),
                None => Error("Index out of bounds")
            },
        }
    }
}