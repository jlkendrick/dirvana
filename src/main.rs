mod utils;

use utils::file_ops;

fn main() {
	let start_dir = std::path::PathBuf::from("..");
	let directories = file_ops::collect_directories(&start_dir);

	for directory in directories {
		println!("{}", directory);
	}
}