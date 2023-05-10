# Вариант 9

 Базовое задание - разработать средствами языка программирования Java распределенное приложение, имитирующее работу соты мобильной телефонной связи при передаче коротких текстовых сообщений (SMS). Сота состоит из одной базовой станции и произвольного количества мобильных телефонных трубок и реализует две операции:

- регистрация трубки в соте (в базовой станции);
- пересылка строки текста от трубки к трубке через базовую станцию.

Имитатор базовой станции естественно реализовать в виде сервера. С имитаторами трубок ситуация сложнее: при регистрации в базовой станции и отсылке сообщений через станцию они играют роль клиентов, а при принятии сообщений от станции - роль серверов. Ниже приводится пример реализации таких имитаторов средствами CORBA. 