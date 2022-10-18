use std::fmt::{Display, Formatter};
use std::hash::Hash;
use std::rc::Rc;

use im::HashMap;
use im::Vector;

use crate::{TesslaBool, TesslaDisplay, TesslaInt, TesslaList, TesslaValue};
use crate::TesslaValue::*;

pub type TesslaMap<T, U> = TesslaValue<HashMap<T, U>>;

impl<T: Display + Hash + Eq, U: Display> TesslaDisplay for HashMap<T, U> {
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_str("Map(")?;
        let mut entries = Vec::from_iter(self.iter()
            .map(|(key, value)| format!("{} -> {}", key, value)));
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

impl<K: Clone + Eq + Hash, V: Clone> TesslaMap<K, V> {
    #[inline]
    pub fn empty() -> TesslaMap<K, V> {
        Value(HashMap::new())
    }

    #[inline]
    pub fn add(&self, key: K, value: V) -> TesslaMap<K, V> {
        match self {
            Error(error) => Error(error),
            Value(map) => {
                Value(map.update(key, value))
            }
        }
    }

    #[inline]
    pub fn contains(&self, key: K) -> TesslaBool {
        match self {
            Error(error) => Error(error),
            Value(map) => Value(map.contains_key(&key)),
        }
    }

    #[inline]
    pub fn fold<A>(&self, start: TesslaValue<A>, function: TesslaValue<Rc<dyn Fn(TesslaValue<A>, K, V) -> TesslaValue<A>>>) -> TesslaValue<A> {
        match (self, function) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(map), Value(function)) => {
                let mut result: TesslaValue<A> = start;
                for (key, value) in map.iter() {
                    result = function(result, key.clone(), value.clone());
                }
                result
            }
        }
    }

    #[inline]
    pub fn keys(&self) -> TesslaList<K> {
        match self {
            Error(error) => Error(error),
            Value(map) => {
                let mut keys = Vector::new();
                for key in map.keys() {
                    keys.push_back(key.clone());
                }
                Value(keys)
            }
        }
    }

    #[inline]
    pub fn remove(&self, key: K) -> TesslaMap<K, V> {
        match self {
            Error(error) => Error(error),
            Value(value) => {
                Value(value.without(&key))
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
    pub fn map<A: Clone + Eq + Hash, B: Clone>(&self, function: TesslaValue<Rc<dyn Fn(K, V) -> TesslaValue<(A, B)>>>) -> TesslaMap<A, B> {
        match (self, function) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(map), Value(function)) => {
                let mut result = HashMap::new();
                for (key, value) in map.iter() {
                    match function(key.clone(), value.clone()) {
                        Error(error) => return Error(error),
                        Value((mapped_key, mapped_value)) => result.insert(mapped_key, mapped_value)
                    };
                }
                return Value(result);
            }
        }
    }
}

impl<K: Clone + Eq + Hash, V: Clone> TesslaMap<TesslaValue<K>, TesslaValue<V>> {
    #[inline]
    pub fn get(&self, key: TesslaValue<K>) -> TesslaValue<V> {
        match self {
            Error(error) => Error(error),
            Value(map) => match map.get(&key) {
                None => Error("Map does not contain the given key"),
                Some(value) => value.clone(),
            },
        }
    }
}