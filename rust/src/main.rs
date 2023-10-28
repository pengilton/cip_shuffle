use std::{error::Error, process};
use std::time::Instant; // Might do {Duration, Instant}
use chrono::{Datelike, Local, Timelike};
use rip_shuffle::scatter_shuffle::sequential::{NUM_BUCKETS, BASE_CASE_SIZE};
use std::path::Path;
use rip_shuffle::RipShuffleSequential;

// This is used to generate our filename
fn get_path_string(num_buckets: usize, threshold: usize) -> String {
    // filename: yyyymmdd-hhmmss-nb=4-th=256-r.csv
    let now = Local::now();

    let path_to_dir = "../benchmarks/rust";

    let mut filename: String = format!("{:0>4}", now.year().to_string());
    filename.push_str(&format!("{:0>2}", now.month().to_string()));
    filename.push_str(&format!("{:0>2}", now.day().to_string()));
    filename.push_str("-");
    filename.push_str(&format!("{:0>2}", now.hour().to_string()));
    filename.push_str(&format!("{:0>2}", now.minute().to_string()));
    filename.push_str(&format!("{:0>2}", now.second().to_string()));
    filename.push_str("-");
    filename.push_str(&format!("nb={}", num_buckets));
    filename.push_str("-");
    filename.push_str(&format!("th={}", threshold));
    filename.push_str("-");
    filename.push_str("r");
    filename.push_str(".csv");

    format!("{0}/{1}", path_to_dir, filename)
}

fn run() -> Result<(), Box<dyn Error>> {
    let path_string = get_path_string(NUM_BUCKETS, BASE_CASE_SIZE);
    let path = Path::new(&path_string);
    let mut wtr = csv::Writer::from_path(path)?;
    
    // Header
    wtr.write_record(&["buckets", "threshold", "run", "integers", "duration [ns]"])?;

    // Let's run our code and store the stuff like in c++
    const  RUNS: usize = 10;
    const  MIN_EXP: usize = 0;    // inclusive
    const  MAX_EXP: usize = 30;   // exclusive

    println!("Starting benchmark with {} buckets...", NUM_BUCKETS);

    for exp in MIN_EXP..MAX_EXP {
        let size: usize = 1 << exp;
        println!("Setting size = {}", size);

        for run in 0..(RUNS + 1) {
            let mut data: Vec<_> = (0..size).into_iter().collect();

            let start = Instant::now();
            data.seq_shuffle(&mut rand::thread_rng());
            let duration = start.elapsed().as_nanos();

            if run > 0 {
                wtr.serialize((NUM_BUCKETS, BASE_CASE_SIZE, run, size, duration))?;
                println!("{0}/{1} Runtime is {2} ns", run, RUNS, duration);
            }
        }
    }

    wtr.flush()?;
    Ok(())
}

fn main() {
    if let Err(_err) = run() {
        
        process::exit(1);
    }
}