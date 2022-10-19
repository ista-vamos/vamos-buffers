// This Rust code was automatically created by tessla-compiler from a TeSSLa Specification
// #![allow(unused_parens, unused_variables, non_snake_case, non_camel_case_types, uncommon_codepoints, non_upper_case_globals)]

extern crate tessla_stdlib;

use std::borrow::BorrowMut;
use std::marker::PhantomData;

use std::os::raw::c_int;
use std::os::raw::c_long;
use tessla_stdlib::*;

pub struct State<
    E,
    Fbalancemismatch,
    BFbalancemismatch,
    Fbalances,
    BFbalances,
    Fbalancenegative,
    BFbalancenegative,
> where
    Fbalancemismatch:
        (FnMut(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), E>) + ?Sized,
    BFbalancemismatch: BorrowMut<Fbalancemismatch>,
    Fbalances: (FnMut(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), E>) + ?Sized,
    BFbalances: BorrowMut<Fbalances>,
    Fbalancenegative:
        (FnMut(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), E>) + ?Sized,
    BFbalancenegative: BorrowMut<Fbalancenegative>,
{
    current_ts: i64,
    last_processed_ts: i64,
    _marker: PhantomData<E>,
    out_balancemismatch: BFbalancemismatch, /* balancemismatch */
    out_balances: BFbalances,               /* balances */
    out_balancenegative: BFbalancenegative, /* balancenegative */
    var_1226_: EventContainer<TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>>,
    var_1174_: EventContainer<TesslaUnit>,
    var_1176_: EventContainer<TesslaUnit>,
    var_1178_: EventContainer<TesslaUnit>,
    var_1180_: EventContainer<TesslaUnit>,
    var_any_1173_: EventContainer<TesslaUnit>,
    var_1223_: EventContainer<TesslaMap<TesslaInt, TesslaInt>>,
    var_balancemismatch_1170_: EventContainer<TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>>,
    _marker_balancemismatch: PhantomData<Fbalancemismatch>,
    var_adjdeposit_1186_: EventContainer<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_adjwithdraw_1188_: EventContainer<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>>,
    var_1190_: EventContainer<TesslaMap<TesslaInt, TesslaInt>>,
    var_balances_1172_: EventContainer<TesslaMap<TesslaInt, TesslaInt>>,
    _marker_balances: PhantomData<Fbalances>,
    var_balancehistory_1222_:
        EventContainer<TesslaMap<TesslaInt, TesslaValue<(TesslaInt, TesslaInt)>>>,
    var_1225_: EventContainer<TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>>,
    var_1229_: EventContainer<
        TesslaValue<
            Rc<
                dyn Fn(
                    TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
                    TesslaInt,
                    TesslaValue<(TesslaInt, TesslaInt)>,
                ) -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
            >,
        >,
    >,
    var_1228_: EventContainer<
        TesslaValue<
            Rc<
                dyn Fn(
                    TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
                    TesslaInt,
                    TesslaValue<(TesslaInt, TesslaInt)>,
                ) -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
            >,
        >,
    >,
    var_balancenegativeOpt_1184_: EventContainer<TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>>,
    var_1185_: EventContainer<TesslaBool>,
    var_balancenegative_1183_: EventContainer<TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>>,
    _marker_balancenegative: PhantomData<Fbalancenegative>,
    var_1191_: EventContainer<TesslaMap<TesslaInt, TesslaInt>>,
    var_transfer: EventContainer<TesslaValue<(TesslaInt, TesslaInt, TesslaInt, TesslaBool)>>,
    var_deposit: EventContainer<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_balance: EventContainer<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_withdraw: EventContainer<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>>,
    var_reset: EventContainer<TesslaUnit>,
}

fn var_getOrElse_1204_<K_525_: 'static + Eq + Hash + Clone, V_526_: 'static + Clone>(
    var_map_1344_: TesslaMap<TesslaValue<K_525_>, TesslaValue<V_526_>>,
    var_key_1345_: TesslaValue<K_525_>,
    var_default_1346_: TesslaValue<V_526_>,
) -> TesslaValue<V_526_> {
    let var_1347_ = match /* if */ (var_map_1344_.clone().contains(var_key_1345_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ var_map_1344_.clone().get(var_key_1345_.clone()) },
    Value(false) => { /* else */ var_default_1346_.clone() }
};
    return var_1347_.clone();
}
fn var_1224__1360_(
    var_lbmp_1351_: TesslaOption<TesslaMap<TesslaInt, TesslaInt>>,
) -> TesslaValue<
    Rc<
        dyn Fn(
            TesslaInt,
            TesslaInt,
        ) -> TesslaValue<(TesslaInt, TesslaValue<(TesslaInt, TesslaInt)>)>,
    >,
> {
    return Value(Rc::new(
        move |var_acc_1361_: TesslaInt, var_bal_1362_: TesslaInt| {
            let var___2_1365_ = var_getOrElse_1204_(
                var_lbmp_1351_.clone().get_some(),
                var_acc_1361_.clone(),
                var_bal_1362_.clone(),
            );
            let var_1364_ = Value((var_bal_1362_.clone(), var___2_1365_.clone()));
            let var_1363_ = Value((var_acc_1361_.clone(), var_1364_.clone()));
            return var_1363_.clone();
        },
    ));
}
fn var_isSome_1201_<T_369_: 'static + Clone>(
    var_opt_1341_: TesslaOption<TesslaValue<T_369_>>,
) -> TesslaBool {
    let var_1342_ = !(var_opt_1341_.clone().is_none());
    return var_1342_.clone();
}
fn var_1192__1288_(
    var_dep_1262_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
) -> TesslaValue<Rc<dyn Fn(TesslaInt, TesslaInt) -> TesslaValue<(TesslaInt, TesslaInt)>>> {
    return Value(Rc::new(
        move |var_acc_1312_: TesslaInt, var_bal_1313_: TesslaInt| {
            let var_1317_ = Lazy::new(|| var_dep_1262_.clone().get_some());
            let var_1314_ = match /* if */ (var_acc_1312_.clone().eq(&match var_1317_.clone() { Value(value) => value.0, Error(error) => Error(error) })) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value((var_acc_1312_.clone(), (var_bal_1313_.clone() + match var_1317_.clone() { Value(value) => value.1, Error(error) => Error(error) }))) },
    Value(false) => { /* else */ Value((var_acc_1312_.clone(), var_bal_1313_.clone())) }
};
            return var_1314_.clone();
        },
    ));
}
fn var_1192__1324_(
    var_wdrw_1263_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>>,
) -> TesslaValue<Rc<dyn Fn(TesslaInt, TesslaInt) -> TesslaValue<(TesslaInt, TesslaInt)>>> {
    return Value(Rc::new(
        move |var_acc_1330_: TesslaInt, var_bal_1331_: TesslaInt| {
            let var_1335_ = Lazy::new(|| var_wdrw_1263_.clone().get_some());
            let var_1332_ = match /* if */ (var_acc_1330_.clone().eq(&match var_1335_.clone() { Value(value) => value.0, Error(error) => Error(error) })) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value((var_acc_1330_.clone(), (var_bal_1331_.clone() - match var_1335_.clone() { Value(value) => value.1, Error(error) => Error(error) }))) },
    Value(false) => { /* else */ Value((var_acc_1330_.clone(), var_bal_1331_.clone())) }
};
            return var_1332_.clone();
        },
    ));
}
fn var_1192__1286_(
    var_wdrw_1263_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>>,
    var_dep_1262_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
) -> TesslaValue<Rc<dyn Fn(TesslaInt, TesslaInt) -> TesslaValue<(TesslaInt, TesslaInt)>>> {
    return Value(Rc::new(
        move |var_acc_1293_: TesslaInt, var_bal_1294_: TesslaInt| {
            let var_1311_ = Lazy::new(|| var_wdrw_1263_.clone().get_some());
            let var_1298_ = Lazy::new(|| var_dep_1262_.clone().get_some());
            let var_1295_ = match /* if */ (var_acc_1293_.clone().eq(&match var_1298_.clone() { Value(value) => value.0, Error(error) => Error(error) })) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value((var_acc_1293_.clone(), (var_bal_1294_.clone() + match var_1298_.clone() { Value(value) => value.1, Error(error) => Error(error) }))) },
    Value(false) => { /* else */ match /* if */ (var_acc_1293_.clone().eq(&match var_1311_.clone() { Value(value) => value.0, Error(error) => Error(error) })) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value((var_acc_1293_.clone(), (var_bal_1294_.clone() - match var_1311_.clone() { Value(value) => value.1, Error(error) => Error(error) }))) },
    Value(false) => { /* else */ Value((var_acc_1293_.clone(), var_bal_1294_.clone())) }
} }
};
            return var_1295_.clone();
        },
    ));
}
fn var_1192_(
    var_dep_1262_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_wdrw_1263_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>>,
    var_blnc_1264_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_rst_1265_: TesslaOption<TesslaUnit>,
    var_bmp_1266_: TesslaOption<TesslaMap<TesslaInt, TesslaInt>>,
) -> TesslaOption<TesslaMap<TesslaInt, TesslaInt>> {
    let var_1274_ = Lazy::new(|| var_isSome_1201_(var_wdrw_1263_.clone()));
    let var_1287_ = Lazy::new(|| var_bmp_1266_.clone().get_some());
    let var_1327_ = Lazy::new(|| var_blnc_1264_.clone().get_some());
    let var_1267_ = match /* if */ (var_isSome_1201_(var_dep_1262_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ match /* if */ (var_1274_.clone()) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(var_1287_.clone().map(var_1192__1286_(var_wdrw_1263_.clone(), var_dep_1262_.clone())))) },
    Value(false) => { /* else */ Value(Some(var_1287_.clone().map(var_1192__1288_(var_dep_1262_.clone())))) }
} },
    Value(false) => { /* else */ match /* if */ (var_1274_.clone()) {
    Error(error) => Error(error),
    Value(true) => { /* then */ match /* if */ (match var_wdrw_1263_.clone().get_some() { Value(value) => value.2, Error(error) => Error(error) }) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(var_1287_.clone().map(var_1192__1324_(var_wdrw_1263_.clone())))) },
    Value(false) => { /* else */ var_bmp_1266_.clone() }
} },
    Value(false) => { /* else */ match /* if */ (var_isSome_1201_(var_blnc_1264_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(var_1287_.clone().add(match var_1327_.clone() { Value(value) => value.0, Error(error) => Error(error) },match var_1327_.clone() { Value(value) => value.1, Error(error) => Error(error) }))) },
    Value(false) => { /* else */ match /* if */ (var_isSome_1201_(var_rst_1265_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(TesslaMap::empty())) },
    Value(false) => { /* else */ var_bmp_1266_.clone() }
} }
} }
} }
};
    return var_1267_.clone();
}
fn var_1189_(
    var_dep_1247_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>>,
    var_trnsfr_1248_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaInt, TesslaBool)>>,
) -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>> {
    let var___1_1254_ = Lazy::new(|| var_trnsfr_1248_.clone().get_some());
    let var_1249_ = match /* if */ (var_isSome_1201_(var_dep_1247_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ var_dep_1247_.clone() },
    Value(false) => { /* else */ match /* if */ (var_isSome_1201_(var_trnsfr_1248_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(Value((match var___1_1254_.clone() { Value(value) => value.0, Error(error) => Error(error) }, match var___1_1254_.clone() { Value(value) => value.2, Error(error) => Error(error) }, match var___1_1254_.clone() { Value(value) => value.3, Error(error) => Error(error) })))) },
    Value(false) => { /* else */ Value(None) }
} }
};
    return var_1249_.clone();
}
fn var_1182_(
    var_bal_1193_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_bmp_1194_: TesslaOption<TesslaMap<TesslaInt, TesslaInt>>,
) -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>> {
    let var_1220_ = Lazy::new(|| var_bal_1193_.clone().get_some());
    let var_1205_ = Lazy::new(|| var_bmp_1194_.clone().get_some());
    let var_1213_ = Lazy::new(|| match var_1220_.clone() {
        Value(value) => value.0,
        Error(error) => Error(error),
    });
    let var_1215_ = Lazy::new(|| match var_1220_.clone() {
        Value(value) => value.1,
        Error(error) => Error(error),
    });
    let var_1200_ = Lazy::new(|| Value(None));
    let var_1195_ = match /* if */ (match (var_isSome_1201_(var_bal_1193_.clone())) {
    Value(true) => { /* and */ var_isSome_1201_(var_bmp_1194_.clone()) },
    false_or_error => false_or_error
}) {
    Error(error) => Error(error),
    Value(true) => { /* then */ match /* if */ (var_getOrElse_1204_(var_1205_.clone(), var_1213_.clone(), var_1215_.clone()).eq(&var_1215_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ var_1200_.clone() },
    Value(false) => { /* else */ Value(Some(Value((var_1213_.clone(), var_1215_.clone(), var_1205_.clone().get(var_1213_.clone()))))) }
} },
    Value(false) => { /* else */ var_1200_.clone() }
};
    return var_1195_.clone();
}
fn var_1230_(
    var_acc_1367_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_k_1368_: TesslaInt,
    var_v_1369_: TesslaValue<(TesslaInt, TesslaInt)>,
) -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>> {
    let var_1374_ = Lazy::new(|| match var_v_1369_.clone() {
        Value(value) => value.0,
        Error(error) => Error(error),
    });
    let var_1370_ = match /* if */ (match (var_1374_.clone().lt(&Value(0_i64))) {
    Value(true) => { /* and */ match var_v_1369_.clone() { Value(value) => value.1, Error(error) => Error(error) }.ge(&Value(0_i64)) },
    false_or_error => false_or_error
}) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(Value((var_k_1368_.clone(), var_1374_.clone())))) },
    Value(false) => { /* else */ var_acc_1367_.clone() }
};
    return var_1370_.clone();
}
fn var_1187_(
    var_dep_1231_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
    var_trnsfr_1232_: TesslaOption<TesslaValue<(TesslaInt, TesslaInt, TesslaInt, TesslaBool)>>,
) -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>> {
    let var___1_1242_ = Lazy::new(|| var_trnsfr_1232_.clone().get_some());
    let var_1238_ = Lazy::new(|| Value(None));
    let var_1233_ = match /* if */ (var_isSome_1201_(var_dep_1231_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ var_dep_1231_.clone() },
    Value(false) => { /* else */ match /* if */ (var_isSome_1201_(var_trnsfr_1232_.clone())) {
    Error(error) => Error(error),
    Value(true) => { /* then */ match /* if */ (match var___1_1242_.clone() { Value(value) => value.3, Error(error) => Error(error) }) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(Value((match var___1_1242_.clone() { Value(value) => value.1, Error(error) => Error(error) }, match var___1_1242_.clone() { Value(value) => value.2, Error(error) => Error(error) })))) },
    Value(false) => { /* else */ var_1238_.clone() }
} },
    Value(false) => { /* else */ var_1238_.clone() }
} }
};
    return var_1233_.clone();
}
fn var_1224_(
    var_bmp_1350_: TesslaOption<TesslaMap<TesslaInt, TesslaInt>>,
    var_lbmp_1351_: TesslaOption<TesslaMap<TesslaInt, TesslaInt>>,
) -> TesslaOption<TesslaMap<TesslaInt, TesslaValue<(TesslaInt, TesslaInt)>>> {
    let var_1352_ = match /* if */ (match (var_isSome_1201_(var_bmp_1350_.clone())) {
    Value(true) => { /* and */ var_isSome_1201_(var_lbmp_1351_.clone()) },
    false_or_error => false_or_error
}) {
    Error(error) => Error(error),
    Value(true) => { /* then */ Value(Some(var_bmp_1350_.clone().get_some().map(var_1224__1360_(var_lbmp_1351_.clone())))) },
    Value(false) => { /* else */ Value(None) }
};
    return var_1352_.clone();
}

impl<
        E,
        Fbalancemismatch,
        BFbalancemismatch,
        Fbalances,
        BFbalances,
        Fbalancenegative,
        BFbalancenegative,
    >
    State<
        E,
        Fbalancemismatch,
        BFbalancemismatch,
        Fbalances,
        BFbalances,
        Fbalancenegative,
        BFbalancenegative,
    >
where
    Fbalancemismatch:
        (FnMut(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), E>) + ?Sized,
    BFbalancemismatch: BorrowMut<Fbalancemismatch>,
    Fbalances: (FnMut(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), E>) + ?Sized,
    BFbalances: BorrowMut<Fbalances>,
    Fbalancenegative:
        (FnMut(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), E>) + ?Sized,
    BFbalancenegative: BorrowMut<Fbalancenegative>,
{
    pub fn new(
        out_balancemismatch: BFbalancemismatch, /* balancemismatch */
        out_balances: BFbalances,               /* balances */
        out_balancenegative: BFbalancenegative, /* balancenegative */
    ) -> Self {
        Self {
            current_ts: 0,
            last_processed_ts: 0,
            _marker: PhantomData,
            var_1226_: init(),
            var_1174_: init(),
            var_1176_: init(),
            var_1178_: init(),
            var_1180_: init(),
            var_any_1173_: init(),
            var_1223_: init(),
            var_balancemismatch_1170_: init(),
            out_balancemismatch: out_balancemismatch,
            _marker_balancemismatch: PhantomData,
            var_adjdeposit_1186_: init(),
            var_adjwithdraw_1188_: init(),
            var_1190_: init(),
            var_balances_1172_: init(),
            out_balances: out_balances,
            _marker_balances: PhantomData,
            var_balancehistory_1222_: init(),
            var_1225_: init_with_value(Value(None)),
            var_1229_: init(),
            var_1228_: init_with_value(TesslaValue::wrap(var_1230_ as fn(_, _, _) -> _)),
            var_balancenegativeOpt_1184_: init(),
            var_1185_: init(),
            var_balancenegative_1183_: init(),
            out_balancenegative: out_balancenegative,
            _marker_balancenegative: PhantomData,
            var_1191_: init_with_value(TesslaMap::empty()),
            var_transfer: init(),
            var_deposit: init(),
            var_balance: init(),
            var_withdraw: init(),
            var_reset: init(),
        }
    }

    pub fn get_current_ts(&self) -> i64 {
        self.current_ts
    }

    pub fn set_transfer(
        &mut self,
        value: TesslaValue<(TesslaInt, TesslaInt, TesslaInt, TesslaBool)>,
    ) {
        self.var_transfer.set_event(value);
    }

    pub fn set_deposit(&mut self, value: TesslaValue<(TesslaInt, TesslaInt)>) {
        self.var_deposit.set_event(value);
    }

    pub fn set_balance(&mut self, value: TesslaValue<(TesslaInt, TesslaInt)>) {
        self.var_balance.set_event(value);
    }

    pub fn set_withdraw(&mut self, value: TesslaValue<(TesslaInt, TesslaInt, TesslaBool)>) {
        self.var_withdraw.set_event(value);
    }

    pub fn set_reset(&mut self, value: TesslaUnit) {
        self.var_reset.set_event(value);
    }

    pub fn flush(&mut self) -> Result<(), E> {
        self.step(self.current_ts, true)
    }

    pub fn step(&mut self, new_input_ts: i64, flush: bool) -> Result<(), E> {
        let mut flush_required = flush;

        if new_input_ts > self.current_ts || flush_required {
            let mut do_processing = true;
            while do_processing {
                if self.current_ts == new_input_ts && !flush_required {
                    do_processing = false;
                } else {
                    constant(&mut self.var_1174_, Value(()), &self.var_deposit);
                    constant(&mut self.var_1176_, Value(()), &self.var_withdraw);
                    constant(&mut self.var_1178_, Value(()), &self.var_balance);
                    constant(&mut self.var_1180_, Value(()), &self.var_transfer);
                    merge(
                        &mut self.var_any_1173_,
                        vec![
                            &self.var_1174_,
                            &self.var_1176_,
                            &self.var_1178_,
                            &self.var_1180_,
                            &self.var_reset,
                        ],
                    );
                    last(
                        &mut self.var_1223_,
                        &self.var_balances_1172_,
                        &self.var_any_1173_,
                    );
                    lift2(
                        &mut self.var_balancemismatch_1170_,
                        &self.var_balance,
                        &self.var_1223_,
                        TesslaValue::wrap(var_1182_ as fn(_, _) -> _),
                    );
                    self.var_balancemismatch_1170_
                        .call_output(self.out_balancemismatch.borrow_mut(), self.current_ts)?;
                    lift2(
                        &mut self.var_adjdeposit_1186_,
                        &self.var_deposit,
                        &self.var_transfer,
                        TesslaValue::wrap(var_1187_ as fn(_, _) -> _),
                    );
                    lift2(
                        &mut self.var_adjwithdraw_1188_,
                        &self.var_withdraw,
                        &self.var_transfer,
                        TesslaValue::wrap(var_1189_ as fn(_, _) -> _),
                    );
                    last(&mut self.var_1190_, &self.var_1191_, &self.var_any_1173_);
                    lift5(
                        &mut self.var_balances_1172_,
                        &self.var_adjdeposit_1186_,
                        &self.var_adjwithdraw_1188_,
                        &self.var_balance,
                        &self.var_reset,
                        &self.var_1190_,
                        TesslaValue::wrap(var_1192_ as fn(_, _, _, _, _) -> _),
                    );
                    self.var_balances_1172_
                        .call_output(self.out_balances.borrow_mut(), self.current_ts)?;
                    lift2(
                        &mut self.var_balancehistory_1222_,
                        &self.var_balances_1172_,
                        &self.var_1223_,
                        TesslaValue::wrap(var_1224_ as fn(_, _) -> _),
                    );
                    default(&mut self.var_1225_, &self.var_1226_);
                    default(&mut self.var_1228_, &self.var_1229_);
                    slift3(
                        &mut self.var_balancenegativeOpt_1184_,
                        &self.var_balancehistory_1222_,
                        &self.var_1225_,
                        &self.var_1228_,
                        TesslaValue::wrap(
                            (|tLPar_0: TesslaMap<
                                TesslaInt,
                                TesslaValue<(TesslaInt, TesslaInt)>,
                            >,
                              tLPar_1: TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
                              tLPar_2: TesslaValue<
                                Rc<
                                    dyn Fn(
                                        TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
                                        TesslaInt,
                                        TesslaValue<(TesslaInt, TesslaInt)>,
                                    )
                                        -> TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>,
                                >,
                            >| {
                                return tLPar_0.fold(tLPar_1, tLPar_2);
                            }) as fn(_, _, _) -> _,
                        ),
                    );
                    slift1(
                        &mut self.var_1185_,
                        &self.var_balancenegativeOpt_1184_,
                        TesslaValue::wrap(var_isSome_1201_ as fn(_) -> _),
                    );
                    filter(
                        &mut self.var_balancenegative_1183_,
                        &self.var_balancenegativeOpt_1184_,
                        &self.var_1185_,
                    );
                    self.var_balancenegative_1183_
                        .call_output(self.out_balancenegative.borrow_mut(), self.current_ts)?;
                    default(&mut self.var_1191_, &self.var_balances_1172_);

                    self.var_1226_.update_last();
                    self.var_1174_.update_last();
                    self.var_1176_.update_last();
                    self.var_1178_.update_last();
                    self.var_1180_.update_last();
                    self.var_any_1173_.update_last();
                    self.var_1223_.update_last();
                    self.var_balancemismatch_1170_.update_last();
                    self.var_adjdeposit_1186_.update_last();
                    self.var_adjwithdraw_1188_.update_last();
                    self.var_1190_.update_last();
                    self.var_balances_1172_.update_last();
                    self.var_balancehistory_1222_.update_last();
                    self.var_1225_.update_last();
                    self.var_1229_.update_last();
                    self.var_1228_.update_last();
                    self.var_balancenegativeOpt_1184_.update_last();
                    self.var_1185_.update_last();
                    self.var_balancenegative_1183_.update_last();
                    self.var_1191_.update_last();
                    self.var_transfer.update_last();
                    self.var_deposit.update_last();
                    self.var_balance.update_last();
                    self.var_withdraw.update_last();
                    self.var_reset.update_last();

                    flush_required = flush && (self.current_ts != new_input_ts);
                    self.last_processed_ts = self.current_ts;
                    self.current_ts = new_input_ts;
                }
            }
        } else if new_input_ts < self.current_ts {
            panic!("{}: FATAL: decreasing timestamp received", self.current_ts);
        }
        Ok(())
    }
}

impl Default
    for State<
        (),
        fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
        fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
        fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
        fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
        fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
        fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >
{
    fn default() -> Self {
        Self::new(
            |value, ts| Ok(output_var(value, "balancemismatch", ts, false)),
            |value, ts| Ok(output_var(value, "balances", ts, false)),
            |value, ts| Ok(output_var(value, "balancenegative", ts, false)),
        )
    }
}

#[no_mangle]
extern "C" fn moninit() -> Box<State<
(),
fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
>>
{
    Box::new(State::default())
}

#[no_mangle]
extern "C" fn mondeposit(mut bs : &mut State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >, acc : c_int, amnt : c_int, ts : c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_deposit(Value((Value(acc.into()), Value(amnt.into()))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn monwithdraw(mut bs : &mut State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >, acc : c_int, amnt : c_int, success : c_int, ts : c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_withdraw(Value((Value(acc.into()), Value(amnt.into()), Value(success==1))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn montransfer(mut bs : &mut State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >, src : c_int, tgt : c_int, amnt : c_int, success : c_int, ts : c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_transfer(Value((Value(src.into()), Value(tgt.into()), Value(amnt.into()), Value(success==1))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn monbalance(mut bs : &mut State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >, acc : c_int, amnt : c_int, ts : c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_balance(Value((Value(acc.into()), Value(amnt.into()))));
    bs.flush().expect("Flush failed");
}

#[no_mangle]
extern "C" fn monreset(mut bs : &mut State<
    (),
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaValue<(TesslaInt, TesslaInt, TesslaInt)>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaMap<TesslaInt, TesslaInt>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    fn(TesslaOption<TesslaValue<(TesslaInt, TesslaInt)>>, i64) -> Result<(), ()>,
    >, ts : c_long)
{
    bs.step(ts.into(), false).expect("Step failed");
    bs.set_reset(Value(()));
    bs.flush().expect("Flush failed");
}
