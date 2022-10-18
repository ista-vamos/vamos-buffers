// #![allow(unused_parens, unused_variables, non_snake_case, non_camel_case_types, uncommon_codepoints, non_upper_case_globals)]
extern crate monitor;
extern crate tessla_stdlib;

use std::borrow::BorrowMut;
use std::marker::PhantomData;
use std::mem;

use tessla_stdlib::*;

use monitor::*;

#[no_mangle]
extern "C" fn init() -> Box<State<
(),
fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
>>
{
    Box::new(State::default())
}

#[no_mangle]
extern "C" fn mondeposit(mut bs : Box<State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >>, acc : std::ffi::c_int, amnt : std::ffi::c_int, ts : std::ffi::c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_deposit(Value((Value(acc.into()), Value(amnt.into()))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn monwithdraw(mut bs : Box<State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >>, acc : std::ffi::c_int, amnt : std::ffi::c_int, success : std::ffi::c_int, ts : std::ffi::c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_withdraw(Value((Value(acc.into()), Value(amnt.into()), Value(success==1))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn montransfer(mut bs : Box<State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >>, src : std::ffi::c_int, tgt : std::ffi::c_int, amnt : std::ffi::c_int, success : std::ffi::c_int, ts : std::ffi::c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_transfer(Value((Value(src.into()), Value(tgt.into()), Value(amnt.into()), Value(success==1))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn monbalance(mut bs : Box<State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >>, acc : std::ffi::c_int, amnt : std::ffi::c_int, ts : std::ffi::c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_balance(Value((Value(acc.into()), Value(amnt.into()))));
    bs.flush().expect("Flush failed");
}