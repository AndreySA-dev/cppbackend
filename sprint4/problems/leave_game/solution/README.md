Разместите в этой папке решение задачи


Test examples:

curl -i -H "Content-Type: application/json" -X GET "http://192.168.1.206:8080/api/v1/maps"
curl -i -H "Content-Type: application/json" -X POST "http://192.168.1.206:8080/api/v1/game/join" -d "{\"mapId\": \"map1\", \"userName\": \"Mikki\"}"
curl -i -X GET "http://192.168.1.206:8080/api/v1/game/players" -H "Authorization: Bearer 6516861d89ebfff147bf2eb2b5153ae1"
curl -i -X GET "http://192.168.1.206:8080/api/v1/game/state" -H "Authorization: Bearer 6516861d89ebfff147bf2eb2b5153ae1"
curl -i  -X POST "http://192.168.1.206:8080/api/v1/game/player/action" -H "Content-Type: application/json" -d "{\"move\": \"R\"}" -H "Authorization: Bearer "  
curl -i  -X POST "http://192.168.1.206:8080/api/v1/game/tick"  -H "Content-Type: application/json" -d "{\"timeDelta\":10}"
curl -i  -X GET "http://192.168.1.206:8080/api/v1/game/records"  -H "Content-Type: application/json"

docker run -d --name postgres-container -p 30432:5432 -e TZ=UTC -e POSTGRES_PASSWORD=YP-12345 ubuntu/postgres:14-22.04_beta
psql postgres://postgres:YP-12345@localhost:30432/

docker build -t lg1 .
docker run --rm -it --network host -e GAME_DB_URL=postgres://postgres:YP-12345@localhost:30432/ --entrypoint /bin/bash lg1:latest

docker build -f Dockerfile_dbg  -t lg1_gdb .

/app/game_server --config-file /app/data/config.json --www-root /app/static/


gdb /app/game_server 
run --config-file /app/data/config.json --www-root /app/static/ --tick-period  100