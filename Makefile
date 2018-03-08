src = $(wildcard src/*.cc)
obj = $(src:.cc=.o)
bin = glquad

GLSLANG = glslangValidator

CXXFLAGS = -pedantic -Wall -g
LDFLAGS = -lGL -lGLU -lglut -lGLEW 

$(bin): $(obj) data/spirv/vertex.spv data/spirv/pixel.spv
	$(CXX) -o $@ $(obj) $(LDFLAGS)

data/spirv/vertex.spv: data/test.v.glsl
	$(GLSLANG) -G -V -S vert -o $@ $<

data/spirv/pixel.spv: data/test.f.glsl data/pixel2.f.glsl
	$(GLSLANG) -l -G -V -S frag -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) $(bin) data/spirv/*
