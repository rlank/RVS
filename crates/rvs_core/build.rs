fn main() {
    cxx_build::bridge("src/ffi.rs")
        .std("c++20")
        .compile("rvs_core_cxx");

    println!("cargo:rerun-if-changed=src/ffi.rs");
}
