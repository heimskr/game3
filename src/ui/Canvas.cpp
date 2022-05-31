// Contains code from nanogui and from LearnOpenGL (https://github.com/JoeyDeVries/LearnOpenGL)

#include "ui/Canvas.h"

namespace Game3 {
// 	constexpr static const char *vertexShader = R"(#version 330
// 		uniform vec2 scaleFactor;
// 		uniform vec2 position;
// 		in vec2 vertex;
// 		out vec2 uv;
// 		void main() {
// 			uv = vertex;
// 			vec2 scaledVertex = (vertex * scaleFactor) + position;
// 			gl_Position = vec4(2.0 * scaledVertex.x - 1.0, 1.0 - 2.0 * scaledVertex.y, 0.0, 1.0);
// 		})";

// 	constexpr static const char *fragmentShader = R"(#version 330
// 		uniform sampler2D image;
// 		out vec4 color;
// 		in vec2 uv;
// 		void main() {
// 			color = texture(image, uv);
// 		})";

	Canvas::Canvas(nanogui::Widget *parent_): GLCanvas(parent_) {
		// setBackgroundColor({255, 255, 255, 255});
		setBackgroundColor({0, 0, 50, 255});

		grass = Texture("resources/grass.png");
		grass.bind();
		int dimension = 640 / 32;
		int scale = 32;
		tilemap = std::make_shared<Tilemap>(dimension, dimension, scale, grass.id);

		int i = 0;
		for (int x = 0; x < dimension; ++x)
			for (int y = 0; y < dimension; ++y)
				// (*tilemap)(x, y) = ++i % 50;
				(*tilemap)(x, y) = rand() % 50;

		tilemapRenderer.initialize(tilemap);

		// shader.init("ImageShader", vertexShader, fragmentShader);

		// nanogui::MatrixXu indices(3, 2);
		// indices.col(0) << 0, 1, 2;
		// indices.col(1) << 2, 3, 1;

		// nanogui::MatrixXf vertices(2, 4);
		// vertices.col(0) << 0, 0;
		// vertices.col(1) << 1, 0;
		// vertices.col(2) << 0, 1;
		// vertices.col(3) << 1, 1;

		// shader.bind();
		// shader.uploadIndices(indices);
		// shader.uploadAttrib("vertex", vertices);

	}

	Canvas::~Canvas() {
		// shader.free();
	}

	void Canvas::drawImage(const Texture &texture, const nanogui::Vector2f &screen_pos,
	                       const nanogui::Vector2f &image_offset, const nanogui::Vector2f &image_extent) {
		// const nanogui::Vector3f position3(screen_pos.x(), screen_pos.y(), 0.f);
		// auto model = Eigen::Affine3f(Eigen::Translation3f(position3)).matrix();
		// TODO, maybe
	}

	void Canvas::drawGL() {
		// static int x = 0;
		// std::cout << "drawGL(" << x++ << ")\n";

		tilemapRenderer.onBackBufferResized(width(), height());
		// glEnable(GL_DEPTH_TEST);
		// grass.bind();
		tilemapRenderer.render();
		// glFlush();
		// glDisable(GL_DEPTH_TEST);
	}
}
