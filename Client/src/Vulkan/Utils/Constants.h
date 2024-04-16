#pragma once

namespace Client {
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	constexpr int DEFAULT_SERVER_PORT = 27020;
	constexpr int DEFAULT_WIDTH = 720;
	constexpr int DEFAULT_HEIGHT = 480;
	constexpr int DEFAULT_IMAGE_WIDTH = 720;
	constexpr int DEFAULT_IMAGE_HEIGHT = 480;
	constexpr int DEFAULT_PIXEL_SIZE = 4;
	
	inline uint64_t getCompressedImageSize(int width, int height) {
		return width * height / 2;
	}
}