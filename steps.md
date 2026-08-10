Excelente. El próximo paso es **MODE +o / -o**.

Este es el modo para dar o quitar privilegios de operador dentro de un canal.

```text
+o = dar operator
-o = quitar operator
```

Ejemplo:

```irc
MODE #test +o titi
```

Significa:

```text
Roger le da permisos de operador a Titi en #test.
```

Después Titi podrá hacer cosas de operador:

```irc
KICK
INVITE
MODE
```

Y para quitarle operador:

```irc
MODE #test -o titi
```

---

## Porcentaje ahora

Si `+l` ya está funcionando con el límite bloqueando `JOIN`, estamos más o menos en:

```text
Estimated mandatory completion: 80% to 83%
```

Ya tenemos casi todos los modos importantes:

```text
[x] +i / -i invite-only
[x] JOIN respeta +i
[x] +t / -t topic restriction
[x] TOPIC respeta +t
[x] +k / -k channel key
[x] JOIN respeta +k
[x] +l / -l user limit
[x] JOIN respeta +l
[ ] +o / -o operator privilege
```

Así que `+o` es el último modo obligatorio grande.

---

# MODE +o / -o

## Qué debe hacer

```irc
MODE #test +o titi
```

Debe:

```text
1. Verificar que Roger está en #test
2. Verificar que Roger es operador
3. Buscar a Titi
4. Verificar que Titi está en #test
5. Agregar a Titi como operador
6. Avisar al canal
```

Respuesta esperada:

```irc
:roger!roger@localhost MODE #test +o titi
```

Para quitar operador:

```irc
MODE #test -o titi
```

Respuesta esperada:

```irc
:roger!roger@localhost MODE #test -o titi
```

---

# 1. `applyChannelMode()` necesita manejar `o`

En `MODE +o`, el argumento extra es el nick:

```irc
MODE #test +o titi
```

Entonces:

```text
mode = +o
modeArg = titi
```

Ya tienes `modeArg` por lo de `+k` y `+l`, así que lo usamos.

Dentro de tu `switch (mode[i])`, agrega este `case 'o'`.

Ponlo después de `case 'l':`.

```cpp
		case 'o':
			if (modeArg.empty())
			{
				sendNumeric(client, "461",
					"MODE :Not enough parameters");
				return (false);
			}
			target = findClientByNickname(modeArg);
			if (target == NULL)
			{
				sendNumeric(client, "401",
					modeArg + " :No such nick/channel");
				return (false);
			}
			if (channel->isMember(target) == false)
			{
				sendNumeric(client, "441", modeArg + " " + channelName
					+ " :They aren't on that channel");
				return (false);
			}
			if (sign == '+')
				channel->addOperator(target);
			else if (sign == '-')
				channel->removeOperator(target);
			else
			{
				sendNumeric(client, "472",
					"o :is unknown mode char to me");
				return (false);
			}
			changed = true;
			break ;
```

But this uses a new variable:

```cpp
Client *target;
```

So at the top of `applyChannelMode()`, change:

```cpp
char	sign;
bool	changed;
```

to:

```cpp
char	sign;
bool	changed;
Client	*target;
```

And initialize it:

```cpp
target = NULL;
```

So the top should be:

```cpp
	char	sign;
	bool	changed;
	Client	*target;

	sign = 0;
	changed = false;
	target = NULL;
```

---

# 2. Fix the MODE broadcast for `+o`

At the bottom of `applyChannelMode()`, you probably have this condition:

```cpp
if ((mode.find("+k") != std::string::npos
		|| mode.find("+l") != std::string::npos)
	&& modeArg.empty() == false)
```

Update it to include `+o` and `-o`:

```cpp
if ((mode.find("+k") != std::string::npos
		|| mode.find("+l") != std::string::npos
		|| mode.find("+o") != std::string::npos
		|| mode.find("-o") != std::string::npos)
	&& modeArg.empty() == false)
```

Full block:

```cpp
if (changed)
{
	if ((mode.find("+k") != std::string::npos
			|| mode.find("+l") != std::string::npos
			|| mode.find("+o") != std::string::npos
			|| mode.find("-o") != std::string::npos)
		&& modeArg.empty() == false)
	{
		sendToChannel(channel, ":" + client->getNickname() + "!"
			+ client->getUsername() + "@localhost MODE "
			+ channelName + " " + mode + " " + modeArg, NULL);
	}
	else
	{
		sendToChannel(channel, ":" + client->getNickname() + "!"
			+ client->getUsername() + "@localhost MODE "
			+ channelName + " " + mode, NULL);
	}
}
```

---

# 3. Update `getChannelModes()`?

No. Usually `+o` is not shown as a simple channel-wide mode in `MODE #test`.

These are channel modes:

```text
+i
+t
+k
+l
```

But `+o` is a **user mode inside the channel**, not a general channel status.

So this is okay:

```irc
MODE #test
:ircserv 324 roger #test +itkl
```

It does not need to show who is operator there. The operator status already appears in `NAMES` as `@titi`.

---

# 4. Test `+o`

Start fresh.

## Roger

```irc
PASS pass
NICK roger
USER roger 0 * :roger
JOIN #test
```

## Titi

```irc
PASS pass
NICK titi
USER titi 0 * :titi
JOIN #test
```

Now Titi is not operator.

From Titi, try:

```irc
KICK #test roger :test
```

Expected:

```irc
:ircserv 482 titi #test :You're not channel operator
```

Now from Roger:

```irc
MODE #test +o titi
```

Expected in both clients:

```irc
:roger!roger@localhost MODE #test +o titi
```

Now from Titi:

```irc
KICK #test roger :test
```

Expected:

```irc
:titi!titi@localhost KICK #test roger :test
```

That proves Titi became operator.

---

# 5. Test `-o`

Restart fresh or create another user, because if Titi kicks Roger, Roger leaves the channel.

Better test like this:

Roger and Titi inside `#test`.

From Roger:

```irc
MODE #test +o titi
MODE #test -o titi
```

Expected:

```irc
:roger!roger@localhost MODE #test +o titi
:roger!roger@localhost MODE #test -o titi
```

Now from Titi:

```irc
KICK #test roger :test
```

Expected:

```irc
:ircserv 482 titi #test :You're not channel operator
```

---

# 6. Error tests

Missing nick:

```irc
MODE #test +o
```

Expected:

```irc
:ircserv 461 roger MODE :Not enough parameters
```

No such nick:

```irc
MODE #test +o nobody
```

Expected:

```irc
:ircserv 401 roger nobody :No such nick/channel
```

Nick exists but not in channel:

```irc
MODE #test +o sammy
```

Expected:

```irc
:ircserv 441 roger sammy #test :They aren't on that channel
```

---

After this works, we can mark:

```text
[x] MODE +o gives channel operator privilege
[x] MODE -o removes channel operator privilege
[x] +o lets user use operator commands
[x] -o blocks user from operator commands again
```

After `+o`, the big mandatory command list is basically complete. Then we move to **disconnect cleanup from channels**, which is important because right now if someone closes `nc`, the channel may still keep a stale `Client*`.


__________________________________________________________________________________________________________________________________


Si `MODE +o / -o` funciona, el próximo paso es:

```text
Disconnect cleanup from channels
```

Esto es importante porque ahora, si alguien cierra `nc` o se desconecta, el servidor borra el `Client*` de `_clients`, pero el canal puede seguir guardando ese mismo puntero en:

```cpp
_members
_operators
_invited
```

Eso puede causar bugs o crash después.

## Próximo paso después de +o

Implementar una función tipo:

```cpp
void	removeClientFromChannels(Client *client);
```

Y llamarla dentro de `removeClient()` **antes de borrar el Client**.

La lógica:

```text
For every channel:
    remove client from members
    remove client from operators
    remove client from invited list
    if channel becomes empty:
        delete channel
        remove it from _channels
```

Eso deja el servidor mucho más estable para evaluación.

## Orden después de eso

```text
1. Disconnect cleanup from channels
2. Test partial messages with Ctrl+D
3. Test multiple commands in one packet
4. Choose real IRC client
5. Test with real IRC client
6. Final cleanup: debug cout, small parsing issues, README/progress update
```

Porcentaje si `+o` funciona:

```text
Estimated mandatory completion: 88% to 90%
```

Buen vuelo. Cuando aterrices seguimos con el cleanup de disconnect.



___________________________________________________________________________________________________________________________


Perfecto. Este paso es **disconnect cleanup from channels**. Esto evita que un canal se quede con punteros viejos cuando un cliente se desconecta. En tu código actual los canales guardan `Client*` en members/operators/invited, así que si borramos el `Client` sin sacarlo de los canales, puede quedar un puntero muerto. Esto es importante para evitar crashes más adelante. Tu `ServerChannels.cpp` ya tiene `removeMember()`, `removeOperator()` y `removeInvited()` disponibles en `Channel`, así que lo hacemos limpio.

## Porcentaje

Si `MODE +o / -o` ya funciona, ahora mismo estás aproximadamente en:

```text
88% to 90%
```

Después de este cleanup funcionando:

```text
90% to 92%
```

Lo que quedaría fuerte:

```text
[ ] Test partial messages con Ctrl+D
[ ] Test multiple commands in one packet
[ ] Test con cliente IRC real
[ ] Cleanup final de debug cout
[ ] Pequeños detalles de parsing si hace falta
[ ] README / progress final
```

---

# 1. `Server.hpp`

Agrega esta función privada:

```cpp
void	removeClientFromChannels(Client *client);
```

Ponla cerca de los helpers de canales, por ejemplo:

```cpp
bool	isValidChannelName(const std::string& name) const;
void	sendToChannel(Channel *channel, const std::string& message,
			Client *except);
void	removeClientFromChannels(Client *client);
```

---

# 2. `ServerChannels.cpp`

Agrega esta función al final del archivo o cerca de los helpers de canales:

```cpp
void	Server::removeClientFromChannels(Client *client)
{
	std::map<std::string, Channel*>::iterator	it;
	std::map<std::string, Channel*>::iterator	tmp;
	Channel									*channel;

	it = _channels.begin();
	while (it != _channels.end())
	{
		channel = it->second;
		channel->removeInvited(client);
		if (channel->isMember(client))
			channel->removeMember(client);
		else
			channel->removeOperator(client);
		if (channel->getMemberCount() == 0)
		{
			tmp = it;
			++it;
			delete tmp->second;
			_channels.erase(tmp);
		}
		else
			++it;
	}
}
```

Qué hace:

```text
1. Recorre todos los canales.
2. Quita al cliente de invited.
3. Si era member, lo quita de members.
4. Si era operator, lo quita de operators.
5. Si el canal queda vacío, borra el canal.
```

Esto es seguro en C++98 porque no usamos el retorno de `erase()`.

---

# 3. `ServerNetwork.cpp`

Ahora hay que llamar esa función dentro de `removeClient()` **antes de borrar el Client**.

Tu función puede ser un poco diferente, pero la idea exacta es esta:

```cpp
removeClientFromChannels(client);
delete client;
```

antes de borrar el cliente del map.

Si quieres ir seguro, reemplaza tu `removeClient()` completo por esta versión:

```cpp
void	Server::removeClient(int fd)
{
	std::map<int, Client*>::iterator	clientIt;
	std::vector<struct pollfd>::iterator	fdIt;
	Client								*client;

	clientIt = _clients.find(fd);
	if (clientIt != _clients.end())
	{
		client = clientIt->second;
		removeClientFromChannels(client);
		delete client;
		_clients.erase(clientIt);
	}
	for (fdIt = _fds.begin(); fdIt != _fds.end(); ++fdIt)
	{
		if (fdIt->fd == fd)
		{
			_fds.erase(fdIt);
			break ;
		}
	}
	close(fd);
	std::cout << "Client disconnected with fd " << fd << std::endl;
}
```

Importante: si tu `ServerNetwork.cpp` no tiene estos includes, asegúrate de tenerlos:

```cpp
#include <unistd.h>
#include <iostream>
```

---

# 4. Compilar

```bash
make re
```

Y revisar `send()`:

```bash
grep -R "send(" -n src include
```

Esperado: solo el `send()` real en `ServerNetwork.cpp`.

---

# 5. Tests rápidos

## Test 1 — desconectar usuario de un canal

Roger:

```irc
PASS pass
NICK roger
USER roger 0 * :roger
JOIN #test
```

Titi:

```irc
PASS pass
NICK titi
USER titi 0 * :titi
JOIN #test
```

Ahora cierra la terminal de Titi o haz `Ctrl+C`.

Luego desde Roger:

```irc
PRIVMSG #test :hello after titi left
```

Esperado:

```text
No crash.
No error raro.
Servidor sigue funcionando.
```

---

## Test 2 — canal vacío se borra

Roger:

```irc
PASS pass
NICK roger
USER roger 0 * :roger
JOIN #emptytest
```

Cierra Roger.

Luego crea otro cliente:

```irc
PASS pass
NICK sammy
USER sammy 0 * :sammy
JOIN #emptytest
```

Esperado:

```irc
:sammy!sammy@localhost JOIN #emptytest
:ircserv 353 sammy = #emptytest :@sammy
```

Sammy debe aparecer con `@`, porque el canal viejo se borró y se creó de nuevo.

---

## Test 3 — operador desconectado

Roger crea canal:

```irc
PASS pass
NICK roger
USER roger 0 * :roger
JOIN #test
```

Titi entra:

```irc
PASS pass
NICK titi
USER titi 0 * :titi
JOIN #test
```

Roger se desconecta.

Ahora Titi intenta:

```irc
KICK #test nobody :test
```

Esperado:

```irc
:ircserv 482 titi #test :You're not channel operator
```

No debe crashear. El canal sigue existiendo con Titi como miembro normal.

Nota: más adelante podemos decidir si cuando el operador se va, el siguiente miembro debería volverse operador. No es obligatorio para el subject. Para defensa simple, no lo agregaría ahora.

---

Después de esto, el siguiente paso sería:

```text
1. Test partial messages with Ctrl+D
2. Test multiple commands in one packet
3. Real IRC client test
```
