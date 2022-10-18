use std::fmt::{Display, Formatter};
use std::hash::Hash;
use std::ops::{Add, BitAnd, BitOr, BitXor, Deref, Div, Mul, Neg, Not, Rem, Shl, Shr, Sub};
use std::rc::Rc;

use TesslaValue::*;

#[derive(Hash)]
pub enum TesslaValue<T> {
    Error(&'static str),
    Value(T),
}

impl<T> TesslaValue<T> {
    #[inline]
    pub fn is_value(&self) -> bool {
        matches!(*self, Value(_))
    }

    #[inline]
    pub fn is_error(&self) -> bool {
        matches!(*self, Error(_))
    }
}

impl<T: Copy> TesslaValue<T> {
    #[inline]
    pub fn get_value(&self) -> T {
        match self {
            &Error(error) => panic!("Expected a value, got an error: {}", error),
            Value(value) => *value,
        }
    }
}

impl<T: Clone> TesslaValue<T> {
    #[inline]
    pub fn clone_value(&self) -> T {
        match self {
            &Error(error) => panic!("Expected a value, got an error: {}", error),
            Value(value) => value.clone(),
        }
    }
}

impl<T: Clone> Clone for TesslaValue<T> {
    #[inline]
    fn clone(&self) -> Self {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.clone()),
        }
    }
}

impl<T: PartialEq> Eq for TesslaValue<T> {}

impl<T: PartialEq> PartialEq<Self> for TesslaValue<T> {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Error(_), _) | (_, Error(_)) => false,
            (Value(lvalue), Value(rvalue)) => lvalue.eq(rvalue),
        }
    }
}

pub trait TesslaDisplay {
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result;
}

impl<T: TesslaDisplay> Display for TesslaValue<T> {
    #[inline]
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            &Error(error) => write!(f, "Error: {}", error),
            Value(value) => value.tessla_fmt(f),
        }
    }
}

pub trait TesslaParse {
    fn tessla_parse(s: &str) -> (Result<Self, &'static str>, &str) where Self: Sized;
}

impl<T: TesslaParse> From<&str> for TesslaValue<T> {
    fn from(s: &str) -> Self {
        match T::tessla_parse(s.trim()) {
            (Ok(result), "") => Value(result),
            (Ok(_), _) => Error("Failed to parse value, match not exhaustive"),
            (Err(error), _) => Error(error),
        }
    }
}

// https://users.rust-lang.org/t/callable-struct-on-stable/54689/7
impl<F> Deref for TesslaValue<Rc<F>> {
    type Target = F;

    fn deref(&self) -> &Self::Target {
        match self {
            Value(value) => value,
            Error(error) => panic!("Tried to deref error: {}", error),
        }
    }
}

// --- Number operations ---

// lhs & rhs
impl<T: BitAnd<Output = T>> BitAnd for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn bitand(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.bitand(rvalue))
        }
    }
}

// lhs | rhs
impl<T: BitOr<Output = T>> BitOr for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn bitor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.bitor(rvalue))
        }
    }
}

// lhs ^ rhs
impl<T: BitXor<Output = T>> BitXor for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn bitxor(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.bitxor(rvalue)),
        }
    }
}

// lhs << rhs
impl<T: Shl<Output = T>> Shl for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn shl(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.shl(rvalue)),
        }
    }
}

// lhs >> rhs
impl<T: Shr<Output = T>> Shr for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn shr(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.shr(rvalue)),
        }
    }
}

// lhs + rhs
impl Add for TesslaInt {
    type Output = Self;

    #[inline]
    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.wrapping_add(rvalue))
        }
    }
}

impl Add for TesslaFloat {
    type Output = Self;

    #[inline]
    fn add(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.add(rvalue))
        }
    }
}

// lhs - rhs
impl Sub for TesslaInt {
    type Output = Self;

    #[inline]
    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.wrapping_sub(rvalue))
        }
    }
}

impl Sub for TesslaFloat {
    type Output = Self;

    #[inline]
    fn sub(self, rhs: Self) -> Self::Output {
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.sub(rvalue))
        }
    }
}

// lhs * rhs
impl Mul for TesslaInt {
    type Output = Self;

    #[inline]
    fn mul(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (_, Value(value)) if value == 0_i64 => Error("Division by zero"),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.wrapping_mul(rvalue)),
        }
    }
}

impl Mul for TesslaFloat {
    type Output = Self;

    #[inline]
    fn mul(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.mul(rvalue)),
        }
    }
}

// lhs / rhs
impl Div for TesslaInt {
    type Output = Self;

    #[inline]
    fn div(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (_, Value(value)) if value == 0_i64 => Error("Division by zero"),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.wrapping_div(rvalue)),
        }
    }
}

impl Div for TesslaFloat {
    type Output = Self;

    #[inline]
    fn div(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.div(rvalue)),
        }
    }
}

// lhs % rhs
impl Rem for TesslaInt {
    type Output = Self;

    #[inline]
    fn rem(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (_, Value(value)) if value == 0_i64 => Error("Division by zero"),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.wrapping_rem(rvalue)),
        }
    }
}

impl Rem for TesslaFloat {
    type Output = Self;

    #[inline]
    fn rem(self, rhs: Self) -> Self::Output {
        use TesslaValue::*;
        match (self, rhs) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.rem(rvalue)),
        }
    }
}

// -val
impl<T: Neg<Output = T>> Neg for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn neg(self) -> Self::Output {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.neg()),
        }
    }
}

// !val
impl<T: Not<Output = T>> Not for TesslaValue<T> {
    type Output = Self;

    #[inline]
    fn not(self) -> Self::Output {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.not()),
        }
    }
}

// 5.1 Bool

pub type TesslaBool = TesslaValue<bool>;

impl TesslaDisplay for bool {
    #[inline]
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        self.fmt(f)
    }
}

// 5.2 Comparison

impl<T: PartialEq> TesslaValue<T> {
    // lhs == rhs
    #[inline]
    pub fn eq(&self, other: &Self) -> TesslaBool {
        match (self, other) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.eq(rvalue)),
        }
    }

    // lhs != rhs
    #[inline]
    pub fn ne(&self, other: &Self) -> TesslaBool {
        match (self, other) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.ne(rvalue)),
        }
    }
}

impl<T: PartialOrd> TesslaValue<T> {
    // lhs < rhs
    #[inline]
    pub fn lt(&self, other: &Self) -> TesslaBool {
        match (self, other) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.lt(rvalue)),
        }
    }

    // lhs <= rhs
    #[inline]
    pub fn le(&self, other: &Self) -> TesslaBool {
        match (self, other) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.le(rvalue)),
        }
    }

    // lhs > rhs
    #[inline]
    pub fn gt(&self, other: &Self) -> TesslaBool {
        match (self, other) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.gt(rvalue)),
        }
    }

    // lhs >= rhs
    #[inline]
    pub fn ge(&self, other: &Self) -> TesslaBool {
        match (self, other) {
            (Error(error), _) | (_, Error(error)) => Error(error),
            (Value(lvalue), Value(rvalue)) => Value(lvalue.ge(rvalue)),
        }
    }
}

// 5.3 Integer

pub type TesslaInt = TesslaValue<i64>;

impl TesslaDisplay for i64 {
    #[inline]
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        self.fmt(f)
    }
}

impl From<TesslaFloat> for TesslaInt {
    #[inline]
    fn from(value: TesslaFloat) -> Self {
        match value {
            Error(error) => Error(error),
            Value(value) => Value(value as i64),
        }
    }
}

impl TesslaInt {
    #[inline]
    pub fn abs(&self) -> Self {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.abs())
        }
    }
}

// 5.4 Float

pub type TesslaFloat = TesslaValue<f64>;

impl TesslaDisplay for f64 {
    #[inline]
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        if self.is_infinite() {
            f.write_str("Infinity")
        } else if self.is_nan() {
            f.write_str("NaN")
        } else {
            write!(f, "{:?}", self)
        }
    }
}

impl From<TesslaInt> for TesslaFloat {
    #[inline]
    fn from(value: TesslaInt) -> Self {
        match value {
            Error(error) => Error(error),
            Value(value) => Value(value as f64),
        }
    }
}

impl TesslaFloat {
    #[inline]
    pub fn powf(&self, exponent: TesslaFloat) -> TesslaFloat {
        match (self, exponent) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(base), Value(exponent)) => Value(base.powf(exponent)),
        }
    }

    #[inline]
    pub fn log(&self, base: TesslaFloat) -> TesslaFloat {
        match (self, base) {
            (&Error(error), _) | (_, Error(error)) => Error(error),
            (Value(value), Value(base)) => Value(value.log(base)),
        }
    }

    #[inline]
    pub fn sin(&self) -> TesslaFloat {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.sin()),
        }
    }

    #[inline]
    pub fn cos(&self) -> TesslaFloat {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.cos()),
        }
    }

    #[inline]
    pub fn tan(&self) -> TesslaFloat {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.tan()),
        }
    }

    #[inline]
    pub fn atan(&self) -> TesslaFloat {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.atan()),
        }
    }

    #[inline]
    pub fn abs(&self) -> Self {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.abs())
        }
    }
}

// 5.5 String

pub type TesslaString = TesslaValue<String>;

impl TesslaDisplay for String {
    #[inline]
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        self.fmt(f)
    }
}

impl<T: TesslaDisplay> TesslaValue<T> {
    #[inline]
    pub fn to_tessla_string(&self) -> TesslaString {
        match self {
            &Error(error) => Error(error),
            Value(_) => Value(self.to_string()),
        }
    }
}

impl TesslaString {
    pub fn concat(&self, other: &TesslaString) -> TesslaString {
        match (self, other) {
            (&Error(error), _) | (_, &Error(error)) => Error(error),
            (Value(lhs), Value(rhs)) => Value(lhs.to_owned() + rhs),
        }
    }

    pub fn to_upper(&self) -> Self {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.to_uppercase())
        }
    }

    pub fn to_lower(&self) -> Self {
        match self {
            Error(error) => Error(error),
            Value(value) => Value(value.to_lowercase())
        }
    }
}

// 5.6 Option

pub type TesslaOption<T> = TesslaValue<Option<T>>;

impl<T: Display> TesslaDisplay for Option<T> {
    #[inline]
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Some(value) => write!(f, "Some({})", value),
            None => f.write_str("None"),
        }
    }
}

impl<T> TesslaOption<T> {
    #[inline]
    pub fn is_none(&self) -> TesslaBool {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.is_none()),
        }
    }

    #[inline]
    pub fn is_some(&self) -> TesslaBool {
        match self {
            &Error(error) => Error(error),
            Value(value) => Value(value.is_some()),
        }
    }
}

impl<T: Clone> TesslaOption<TesslaValue<T>> {
    #[inline]
    pub fn get_some(&self) -> TesslaValue<T> {
        match self {
            &Error(error) => Error(error),
            Value(Some(value)) => value.clone(),
            Value(None) => Error("Tried to getSome(None)"),
        }
    }

    #[inline]
    pub fn get_some_or_else(&self, fallback: TesslaValue<T>) -> TesslaValue<T> {
        match self {
            &Error(error) => Error(error),
            Value(Some(value)) => value.clone(),
            Value(None) => fallback,
        }
    }
}

// Unit

pub type TesslaUnit = TesslaValue<()>;

impl TesslaDisplay for () {
    #[inline]
    fn tessla_fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.write_str("()")
    }
}
