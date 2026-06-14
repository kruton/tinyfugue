class Tinyfugue < Formula
  desc "Programmable MUD client"
  homepage "https://github.com/kruton/tinyfugue"
  url "https://github.com/kruton/tinyfugue/archive/02b048204c5ffd1704b48541724b8ecebc9aee4c.tar.gz"
  version "5.0.8"
  sha256 "02d11e70251a823f29f7faf2155d77744a91319125efd21a9d90325b7984725e"
  license "GPL-2.0-or-later"
  head "https://github.com/kruton/tinyfugue.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "icu4c"
  depends_on "ncurses"
  depends_on "openssl@3"
  depends_on "pcre2"
  depends_on "zlib"

  def install
    args = std_cmake_args + %W[
      -DTF_WIDECHAR=ON
      -DTF_TLS=ON
      -DTF_TERMCAP=ON
      -DTF_ZLIB=ON
      -DCMAKE_PREFIX_PATH=#{Formula["icu4c"].opt_prefix};#{Formula["openssl@3"].opt_prefix}
    ]

    system "cmake", "-S", ".", "-B", "build", *args
    system "cmake", "--build", "build"
    system "ctest", "--test-dir", "build", "--output-on-failure"
    system "cmake", "--install", "build"
  end

  test do
    output = shell_output("#{bin}/tf -? 2>&1", 1)
    assert_match "Usage:", output
    assert_path_exists share/"tf-lib/tf-help.idx"
  end
end

