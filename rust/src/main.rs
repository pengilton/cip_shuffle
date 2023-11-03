use std::io;
use std::{error::Error, process};
use std::time::{Duration, Instant}; // Might do {Duration, Instant}
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

struct Benchmark {
    function_name: String,
    prng_name: String,
    num_buckets: usize,
    threshold: usize,
    min_exp: usize,
    max_exp: usize,
    size: usize,
    total_runs: usize,
    total_runtime: u128,
    default_runs: usize,
    min_duration: Duration
}

impl Benchmark {
    fn create_header<W: io::Write>(&self, wtr: &mut csv::Writer<W>) {
        // Ignoring reuslt of error handling
        let _ = wtr.write_record(&["function", "prng", "buckets", "threshold", "min_exp", 
                         "max_exp", "integers", "total_runs", "total_runtime"]);
    }

    // Ignoring reuslt of error handling
    fn write_data<W: io::Write>(&self, wtr: &mut csv::Writer<W>) {
        let _ = wtr.serialize((self.function_name.clone(), self.prng_name.clone(), self.num_buckets, 
                              self.threshold, self.min_exp, self.max_exp, self.size, 
                              self.total_runs, self.total_runtime));
    }
}

fn benchmark_seq_shuffle() -> Result<(), Box<dyn Error>> {
    let mut benchmark = Benchmark {
        function_name: "seq_shuffle".to_owned(),
        prng_name: "thread_rng".to_owned(),
        num_buckets: NUM_BUCKETS,
        threshold: BASE_CASE_SIZE,
        min_exp: 0,
        max_exp: 29,
        size: 0,
        total_runs: 0,
        total_runtime: 0u128,
        default_runs: 10,
        min_duration: Duration::from_millis(100)
    };

    let path_string = get_path_string(NUM_BUCKETS, BASE_CASE_SIZE);
    let path = Path::new(&path_string);
    let mut wtr = csv::Writer::from_path(path)?;
    
    // Header
    benchmark.create_header(&mut wtr);

    println!("Starting benchmark with {} buckets...", benchmark.num_buckets);
    let mut data: Vec<_> = (0..(usize::pow(2, benchmark.max_exp as u32))).into_iter().collect();

    for exp in (benchmark.min_exp)..(benchmark.max_exp + 1) {
        benchmark.size = usize::pow(2, exp as u32);
        println!("Setting size = {}", benchmark.size);
        let data_slie = &mut data[..(benchmark.size)];
        
        benchmark.total_runs = benchmark.default_runs;
        loop {
            let start = Instant::now();
            for _run in 0..(benchmark.total_runs) {
                data_slie.seq_shuffle(&mut rand::thread_rng());
            }
            benchmark.total_runtime = start.elapsed().as_nanos();

            if benchmark.total_runtime > benchmark.min_duration.as_nanos() {
                benchmark.write_data(&mut wtr);
                println!("Runtime is {} ns", benchmark.total_runtime);
                break;
            }
            benchmark.total_runs *= 10;
        }
    }

    wtr.flush()?;
    Ok(())
}

fn main() {
    if let Err(_err) = benchmark_seq_shuffle() {
        process::exit(1);
    }
}