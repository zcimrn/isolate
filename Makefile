all:
	go build
	sudo chown root:root isolate
	sudo chmod 4755 isolate
	mkdir box
