use crate::{parse_struct_inner, TesslaDisplay, TesslaParse, TesslaRecordParse, TesslaValue};
use crate::TesslaValue::*;

macro_rules! unassigned {
    ($val:tt) => {"Value not assigned while parsing"};
}

macro_rules! tuple_impls {
    ($(
        $Tuple:ident {
            $(($idx:tt, $T:ident, $key:literal, $fmt:literal))+
        }
    )+) => {
        $(
            impl<$($T: TesslaDisplay + Clone),+> TesslaDisplay for ($(TesslaValue<$T>,)+) {
                fn tessla_fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                    f.write_str("(")?;
                    $(write!(f, $fmt, self.$idx)?;)+
                    f.write_str(")")
                }
            }
            impl<$($T: TesslaParse + Clone),+> TesslaRecordParse for ($(TesslaValue<$T>,)+) {
                fn tessla_parse_tuple(s: &str) -> (Result<Self, &'static str>, &str) {
                    let mut inner = s.trim_start();
                    let mut i = 0_i64;
                    let mut result = ($(Error(unassigned!($idx)),)+);
                    while !inner.starts_with(")") {
                        match i {
                            $(
                            $idx => if !parse_struct_inner(&mut result.$idx, &mut inner) { break; },
                            )+
                            _ => return (Err("Encountered too many values while parsing Tuple"), inner)
                        }
                        i += 1;
                    }
                    (Ok(result), inner)
                }
                fn tessla_parse_struct(s: &str) -> (Result<Self, &'static str>, &str) {
                    let mut inner = s.trim_start();
                    let mut result = ($(Error(unassigned!($idx)),)+);
                    while !inner.starts_with("}") {
                        match inner.split_once(":").or_else(|| inner.split_once("=")) {
                            Some((lhs, rhs)) => {
                                inner = rhs.trim_start();
                                match lhs.trim() {
                                    $(
                                    $key => if !parse_struct_inner(&mut result.$idx, &mut inner) { break; },
                                    )+
                                    _ => return (Err("Encountered invalid key while parsing Tuple"), inner)
                                }
                            }
                            None => break
                        }
                    }
                    (Ok(result), inner)
                }
            }
        )+
    }
}

tuple_impls! {
    Tuple1 {
        (0, T1, "_1", "{},")
    }
    Tuple2 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}")
    }
    Tuple3 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}")
    }
    Tuple4 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}")
    }
    Tuple5 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}")
    }
    Tuple6 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}")
    }
    Tuple7 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}, ")
        (6, T7, "_7", "{}")
    }
    Tuple8 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}, ")
        (6, T7, "_7", "{}, ")
        (7, T8, "_8", "{}")
    }
    Tuple9 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}, ")
        (6, T7, "_7", "{}, ")
        (7, T8, "_8", "{}, ")
        (8, T9, "_9", "{}")
    }
    Tuple10 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}, ")
        (6, T7, "_7", "{}, ")
        (7, T8, "_8", "{}, ")
        (8, T9, "_9", "{}, ")
        (9, T10, "_10", "{}")
    }
    Tuple11 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}, ")
        (6, T7, "_7", "{}, ")
        (7, T8, "_8", "{}, ")
        (8, T9, "_9", "{}, ")
        (9, T10, "_10", "{}, ")
        (10, T11, "_11", "{}")
    }
    Tuple12 {
        (0, T1, "_1", "{}, ")
        (1, T2, "_2", "{}, ")
        (2, T3, "_3", "{}, ")
        (3, T4, "_4", "{}, ")
        (4, T5, "_5", "{}, ")
        (5, T6, "_6", "{}, ")
        (6, T7, "_7", "{}, ")
        (7, T8, "_8", "{}, ")
        (8, T9, "_9", "{}, ")
        (9, T10, "_10", "{}, ")
        (10, T11, "_11", "{}, ")
        (11, T12, "_12", "{}")
    }
}
