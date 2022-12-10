build/html: source/*
	python3 -m venv env
	env/bin/pip install -r requirements.txt
	env/bin/sphinx-build source build/html

.PHONY: clean
clean:
	rm -rf build env
