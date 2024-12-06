#pragma once

#include <random>

class Noise
{
private:
	Noise() { mt.seed(rnd()); }

public:
	static Noise& instance()
	{
		static Noise instance;
		return instance;
	}
	uint32_t get_rnd() { return static_cast<uint32_t> (mt()); }
	/// <summary>
	/// 最小値と最大値を取りその範囲内でのランダム値を返す
	/// </summary>
	/// <param name="min">最低値</param>
	/// <param name="max">最高値</param>
	/// <returns></returns>
	float random_range(float min, float max);

	/// <summary>
	/// 指定した値のマイナスの値とプラスの値の範囲内でのランダム値を返す
	/// </summary>
	/// <param name="value">指定する値</param>
	/// <returns></returns>
	float random_fixed_range(float value);
private:
	std::random_device rnd;
	std::mt19937 mt;
};



#include <vector>

// THIS CLASS IS A TRANSLATION TO C++11 FROM THE REFERENCE
// JAVA IMPLEMENTATION OF THE IMPROVED PERLIN FUNCTION (see http://mrl.nyu.edu/~perlin/noise/)
// THE ORIGINAL JAVA IMPLEMENTATION IS COPYRIGHT 2002 KEN PERLIN

// I ADDED AN EXTRA METHOD THAT GENERATES A NEW PERMUTATION VECTOR (THIS IS NOT PRESENT IN THE ORIGINAL IMPLEMENTATION)

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

class PerlinNoise {
	// The permutation vector
	std::vector<int> p;
public:
	// Initialize with the reference values for the permutation vector
	PerlinNoise();
	// Generate a new permutation vector based on the value of seed
	PerlinNoise(unsigned int seed);
	// Get a noise value, for 2D images z can have any value
	double noise(double x, double y, double z);
private:
	double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);
};

#endif