use std::fmt::{Display, Formatter};
use std::hash::Hash;
use std::rc::Rc;

use im::HashSet;

use crate::{TesslaBool, TesslaDisplay, TesslaInt, TesslaValue};
use crate::TesslaValue::*;

pub type TesslaSet<T> = TesslaValue<HashSet<T>>;

impl<T: Display> TesslaDisplay for HashSet<T> {
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_str("Set(")?;
        let mut entries = Vec::from_iter(self.iter()
            .map(|value| format!("{}", value)));
        entries.sort_unstable();
        for (i, entry) in entries.iter().enumerate() {
            if i > 0 {
                f.write_str(", ")?;
            }
            f.write_str(entry)?;
        }
        f.write_str(")")
    }
}

impl<T> TesslaSet<T> {
    #[inline]
    pub fn empty() -> TesslaSet<TesslaValue<T>> {
        Value(HashSet::<TesslaValue<T>>::new())
    }
}

impl<T: Clone + Eq + Hash> TesslaSet<TesslaValue<T>> {
    #[inline]
    pub fn add(&self, x: TesslaValue<T>) -> TesslaSet<TesslaValue<T>> {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.update(x))
        }
    }

    #[inline]
    pub fn contains(&self, item: TesslaValue<T>) -> TesslaBool {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.contains(&item))
        }
    }

    #[inline]
    pub fn intersection(&self, set2: TesslaSet<TesslaValue<T>>) -> TesslaSet<TesslaValue<T>> {
        match (self, set2) {
            (Error(error), _) => Error(error),
            (_, Error(error)) => Error(error),
            (Value(set_a), Value(set_b)) =>
                Value(set_a.clone().intersection(set_b))
        }
    }

    #[inline]
    pub fn minus(&self, set2: TesslaSet<TesslaValue<T>>) -> TesslaSet<TesslaValue<T>> {
        match (self, set2) {
            (Error(error), _) => Error(error),
            (_, Error(error)) => Error(error),
            (Value(set_a), Value(set_b)) =>
                Value(set_a.clone().difference(set_b))
        }
    }

    #[inline]
    pub fn remove(&self, item: TesslaValue<T>) -> TesslaSet<TesslaValue<T>> {
        match self {
            Error(error) => Error(error),
            Value(value) =>
                Value(value.without(&item))
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
    pub fn union(&self, set2: TesslaSet<TesslaValue<T>>) -> TesslaSet<TesslaValue<T>> {
        match (self, set2) {
            (Error(error), _) => Error(error),
            (_, Error(error)) => Error(error),
            (Value(set_a), Value(set_b)) =>
                Value(set_a.clone().union(set_b))
        }
    }

    #[inline]
    pub fn fold<U>(&self, start: TesslaValue<U>, function: TesslaValue<Rc<dyn Fn(TesslaValue<U>, TesslaValue<T>) -> TesslaValue<U>>>) -> TesslaValue<U> {
        match (self, function) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(set), Value(function)) => {
                let mut result: TesslaValue<U> = start;
                for item in set.iter() {
                    result = function(result, item.clone());
                }
                return result;
            }
        }
    }

    #[inline]
    pub fn map<U: Clone + Eq + Hash>(&self, function: TesslaValue<Rc<dyn Fn(TesslaValue<T>) -> TesslaValue<U>>>) -> TesslaSet<TesslaValue<U>> {
        match (self, function) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(set), Value(function)) => {
                let mut result = HashSet::new();
                for item in set {
                    result.insert(function(item.clone()));
                }
                return Value(result);
            }
        }
    }
}