mod utils;

use utils::file_ops;

fn main() {
	let start_dir = "..";
	let directories = file_ops::collect_directories(start_dir);

	for directory in directories {
		println!("{}", directory);
	}
}