# arch dependencies
install-deps:
	sudo pacman -S pkg-config gcc go qt6-base
	go install github.com/mappu/miqt/cmd/miqt-uic@latest
	go install github.com/mappu/miqt/cmd/miqt-rcc@latest
	go get github.com/mappu/miqt
	go get github.com/mappu/miqt/qt6


build:
	go build -ldflags '-s -w' ./cmd/pwscp/main.go


run:
	go run -ldflags '-s -w' ./cmd/pwscp/main.go
