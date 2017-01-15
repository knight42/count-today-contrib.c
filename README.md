README
=======

今天 push 了吗?

### Intro

学习使用 CMake 以及 libcurl 的小项目, 获取当天 GitHub 上指定用户的 contribution 数目

PS: 有了 STL 真是轻松愉快加写意 ;-)

### Installation:

```shell
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=~/.local ..
make && make install
```

### Usage:

```shell
$ fetch knight42
5
```
