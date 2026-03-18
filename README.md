# server-chat-using-boost.asio
Apenas um simples server chat async usando boost.asio.
usei como referencia a propria boost.asio.
alguns recursos são essenciais.

por exemplo, steady_time para evitar o error de alocação no codigo.
lembrando, que tem outra solução usando experimental::channel.

para rodar na linha de comando use o seguinte codigo:

g++ -std=c++20 srv.cpp -o srv

suporta outros clientes, recomendo usar telnet. 

para um melhor entedimento, acesse boost.asio examples ou tutorial.

adendo: posso atualizar o codigo para algumas implementações no futuro.
