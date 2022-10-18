use std::io::stdin;
use std::str::FromStr;

use crate::{TesslaDisplay, TesslaValue};

pub fn output_var<T: TesslaDisplay>(output: TesslaValue<T>, true_name: &str, ts: i64, raw: bool) {
    if output.is_error() {
        panic!("{}", output);
    }
    if raw {
        println!("{}", output);
    } else {
        println!("{}: {} = {}", ts, true_name, output);
    }
}

pub fn parse_input(input_stream_name: &mut String, input_stream_value: &mut String, new_input_ts: &mut i64) -> bool {
    let mut line = String::new();
    let mut reached_eof = false;

    match stdin().read_line(&mut line) {
        Err(error) => panic!("ERROR parsing the input stream\n{}", error),
        Ok(0) => reached_eof = true,
        Ok(_) => {
            match line.trim() {
                "" => { return false; } // empty line
                s if s.starts_with("#") => { return false; } // comment
                _ => {}
            };
            let (timestamp, input_expression) = match line.trim().split_once(':') {
                Some(value) => value,
                None => panic!("ERROR parsing input '{}'", line.trim())
            };
            *new_input_ts = match i64::from_str(timestamp.trim()) {
                Ok(value) => value,
                Err(err) => panic!("ERROR failed to parse timestamp:\n{}", err)
            };
            match input_expression.split_once('=') {
                Some((lhs, rhs)) => {
                    *input_stream_name = lhs.trim().to_string();
                    *input_stream_value = rhs.trim().to_string();
                }
                None => {
                    *input_stream_name = input_expression.trim().to_string();
                    *input_stream_value = String::from("()");
                }
            };
        }
    };
    line.clear();

    return reached_eof;
}