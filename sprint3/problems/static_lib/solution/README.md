Разместите в этой папке решение задачи

Test examples:

curl -i -H "Content-Type: application/json" -X GET "http://192.168.1.206:8080/api/v1/maps"
curl -i -H "Content-Type: application/json" -X POST "http://192.168.1.206:8080/api/v1/game/join" -d "{\"userName\": \"Mikki\", \"mapId\": \"map1\"}"
curl -i -X GET "http://192.168.1.206:8080/api/v1/game/players" -H "Authorization: Bearer 6516861d89ebfff147bf2eb2b5153ae1"
curl -i -H "Authorization: Bearer 6516861d89ebfff147bf2eb2b5153ae1" -X GET "http://192.168.1.206:8080/api/v1/game/state"
curl -i  -X POST "http://192.168.1.206:8080/api/v1/game/player/action" -H "Content-Type: application/json" -d "{\"move\": \"R\"}" -H "Authorization: Bearer "  
curl -i  -X POST "http://192.168.1.206:8080/api/v1/game/tick"  -H "Content-Type: application/json" -d "{\"timeDelta\":10}"
curl -i  -X POST "http://192.168.1.206:8080/api/v1/game/tick"  -H "Content-Type: application/json" -d "{\"timeDelta\":10}"