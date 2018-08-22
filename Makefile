.PHONY: deps world

deps:
	@yarn

world: deps
	@bsb -make-world -w
