<h1>Buffer设计要点</h1>

* 使用窗口来控制buffer的可用范围
* 使用readv writev来替代recv(MSG_PEEK)以减少一次系统调用
* 使用std::vector自动增长
* 