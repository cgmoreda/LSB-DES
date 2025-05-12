#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <set>
using namespace std;
vector<int> pc1, pc2, IP_t, IP_1, E_t, P, P_1;
vector<vector<vector<int>>> s;
void print(int64_t x, int depth = 64)
{
		for (int i = depth - 1; i >= 0; i--)
				cout << ((x >> i) & 1);
		cout << endl;
}
void print(vector<bool> x)
{
		for (int i = 0; i < x.size(); i++)
				cout << (int)x[i];
		cout << endl;
}
void permute(int64_t& x, vector<int>& p, int mx = 64)
{
		int64_t ret = 0, j = p.size() - 1;
		for (auto& i : p)
		{
				if (x & (1ll << (mx - i)))
						ret |= (1ll << j);
				j--;
		}
		x = ret;
}
int64_t f(int64_t r, int64_t k)
{
		int64_t E = r, ret = 0;
		permute(E, E_t, 32);
		E ^= k;
		for (int i = 0; i < 48; i += 6)
		{
				int64_t num = (E >> i) & ((1 << 6) - 1);
				int64_t n1 = ((num & (1 << 5)) >> 4) | (num & 1);
				int64_t n2 = (num >> 1) & ((1 << 4) - 1);
				ret |= (s[7 - i / 6][n1][n2] << (4 * (i / 6)));
		}
		permute(ret, P, 32);
		return ret;
}
int64_t DES_enc_block(int64_t plain, vector<int64_t>& k)
{
		permute(plain, IP_t);
		int64_t l = plain >> 32, r = plain & ((1ll << 32) - 1);
		for (int i = 1; i <= 16; i++)
		{
				int64_t f_i = f(r, k[i]);
				int64_t nl = r;
				r = l ^ f_i;
				l = nl;
		}
		l &= (1ll << 32) - 1;
		r &= (1ll << 32) - 1;
		int64_t res = l | (r << 32);
		permute(res, IP_1);
		return res;
}
int64_t DES_dec_block(int64_t block, vector<int64_t>& k)
{
		int64_t l = block & ((1ll << 32) - 1);
		int64_t r = block >> 32;
		cout << "l,r 16<<endl;" << endl;
		print(l, 32);
		print(r, 32);
		for (int i = 16; i >= 1; i--)
		{
				int64_t r2 = l;
				l = r ^ f(l, k[i]);
				r = r2;
		}
		l &= (1ll << 32) - 1;
		r &= (1ll << 32) - 1;
		return l | (r << 32);
}
string KEY = "sometihng";
string PRKEY = "";
vector<int64_t> c(17), d(17), k(17);
void prep(string key)
{
		if (key != PRKEY)
		{
				PRKEY = key;
				int K;//todo  get from key
				int64_t k_ = K;
				k_ = 1383827165325090801;
				permute(k_, pc1);
				c[0] = k_ >> 28;
				d[0] = k_ & ((1ll << 28) - 1);
				auto cyclic_left_shift = [&](int64_t& x)
				{
					x = ((x & (1 << 27)) >> 27) | ((x & ((1 << 28) - 1)) << 1);
				};
				for (int i = 1; i <= 16; i++)
				{
						c[i] = c[i - 1], d[i] = d[i - 1];
						cyclic_left_shift(c[i]), cyclic_left_shift(d[i]);
						if (!(i <= 2 || i == 9 || i == 16))
								cyclic_left_shift(c[i]), cyclic_left_shift(d[i]);
						k[i] = 0;
						for (int j = 0, j2 = 27; j < 28; j2--, j++)
						{
								k[i] |= ((c[i] >> j2) & 1) << (j2 + 28);
								k[i] |= ((d[i] >> j2) & 1) << (j2 + 0);
						}
						// todo try remove the loop
						permute(k[i], pc2, 56);
				}
		}
}
vector<bool> DES_enc(vector<bool>& plain, string const& key = KEY)
{
		prep(key);
		// todo remove extras

		vector<bool> ret(plain.size());
		for (int i = 0; i + 64 <= plain.size(); i += 64)
		{
				int64_t block = 0;
				for (int j = 0; j < 64; j++)
						block |= (1ll * plain[i + j]) << (63 - j);
				block = DES_enc_block(block, k);
				for (int j = 0; j < 64; j++)
						ret[i + 63 - j] = (block >> j) & 1;
		}
		return ret;
}// Function for DES decryption on a single block

vector<bool> DES_dec(vector<bool>& cipher, string const& key = KEY)
{
		prep(key);
		reverse(k.begin() + 1, k.end());
		vector<bool> ret(cipher.size());
		for (int i = 0; i + 64 <= cipher.size(); i += 64)
		{
				int64_t block = 0;
				for (int j = 0; j < 64; j++)
						block |= (1ll * cipher[i + j]) << (63 - j);
				block = DES_enc_block(block, k);
				for (int j = 0; j < 64; j++)
						ret[i + 63 - j] = (block >> j) & 1;
		}
		reverse(k.begin() + 1, k.end());
		return ret;
}
struct IMG
{
		int rows, cols;
		vector<uchar> data;
};
void show(cv::Mat img)
{
		cv::imshow("photo", img);
		cv::waitKey();
}
IMG readColImage(const string& path)
{
		cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
		if (img.empty())
		{
				cerr << "Error: Could not load image at " << path << endl;
				return {};
		}
		return {
				img.rows, img.cols, vector<uchar>(img.data, img.data + img.total() * img.channels())
		};
}
void writeColImage(const string& path, IMG ig)
{
		cv::Mat img(ig.rows, ig.cols, CV_8UC3, const_cast<uchar*>(ig.data.data()));
		if (!cv::imwrite(path, img))
		{
				cerr << "Error: Could not save image to " << path << endl;
		}
		else
		{
				cout << "Image successfully saved to " << path << endl;
		}
}
void writeGrayImage(const string& path, IMG ig)
{
		// Create a cv::Mat object for a grayscale image (CV_8UC1 indicates single-channel grayscale)
		cv::Mat img(ig.rows, ig.cols, CV_8UC1, const_cast<uchar*>(ig.data.data()));

		// Save the image
		if (!cv::imwrite(path, img))
		{
				cerr << "Error: Could not save image to " << path << endl;
		}
		else
		{
				cout << "Image successfully saved to " << path << endl;
		}
}
IMG readBWImage(const string& path)
{
		cv::Mat img = cv::imread(path, cv::IMREAD_GRAYSCALE);
		uchar* data = img.data;
		vector<uchar> ret;
		int sz = img.total() * img.channels();
		for (int i = 0; i < sz; ++i)
				ret.push_back(data[i]);
		return { img.rows, img.cols, ret };
}
IMG EncryptGrayScaleInRGB(string pathGrayscale, string pathRGB)
{
		IMG imgBW = readBWImage(pathGrayscale);
		IMG imgRGB = readColImage(pathRGB);
		vector<bool> stream;
		for (int bt = 0; bt < 32; bt++)
				stream.push_back((imgBW.rows & (1 << bt)) > 0);
		for (int bt = 0; bt < 32; bt++)
				stream.push_back((imgBW.cols & (1 << bt)) > 0);
		for (auto& pix : imgBW.data)
				for (int bt = 0; bt < 8; bt++)
						stream.push_back((pix >> bt) & 1);
		while (stream.size() % 64)
				stream.push_back(0);
		if (stream.size() > imgRGB.data.size())
		{
				cerr << stream.size() << endl;
				cerr << imgRGB.data.size() << endl;
				cerr << "RGB image too small" << endl;
				return {};
		}
		stream = DES_enc(stream);
		int j = 0;
		for (auto& i :



		)
		{
				i = (i ^ (i & 1)) | stream[j++];
				if (j == stream.size())
						break;
		}
		return imgRGB;
}
IMG DecryptGrayScaleFromRGB(string pathImg)
{
		IMG img = readColImage(pathImg);
		vector<bool> stream;
		for (int bt = 0; bt < 64; bt++)
				stream.push_back(img.data[bt] & 1);
		stream = DES_dec(stream);
		int rows = 0, cols = 0;
		for (int i = 0; i < 32; i++)
		{
				rows |= stream[i] << i;
				cols |= stream[i + 32] << i;
		}
		stream.clear();
		int dataBits = rows * cols * 8;
		int paddedBits = dataBits + (64 - (dataBits % 64)) % 64;
		for (int i = 64; i < paddedBits; i++)
				stream.push_back(img.data[i] & (1));
		stream = DES_dec(stream);
		vector<uchar> data(rows * cols);
		for (int i = 0; i < dataBits; i++)
				data[i / 8] |= (stream[i] << (i % 8));
		return { rows, cols, data };
}
void init();
void testDES()
{
		vector<bool> plain = {
				1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1,
				0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
				1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1,
				1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1
		};
		print(plain);
		vector<bool> cipher = DES_enc(plain);
		print(cipher);
		plain = DES_dec(cipher);
		print(plain);
};
int main()
{
		init();
		//testDES();
		string path1 = "/mnt/c/Users/mhass/Desktop/c++ ultimate/untitled/kitten.png";
		string path2 = "/mnt/c/Users/mhass/Desktop/c++ ultimate/untitled/MOmeow1_square.png";
		string pathExportedImage = "/mnt/c/Users/mhass/Desktop/c++ ultimate/untitled/export.png";
		string path4 = "/mnt/c/Users/mhass/Desktop/c++ ultimate/untitled/extracted.png";
		IMG injected = EncryptGrayScaleInRGB(path1, path2);
		if (injected.data.size())
		{
				writeColImage(pathExportedImage, injected);
				IMG extracted = DecryptGrayScaleFromRGB(pathExportedImage);
				writeGrayImage(path4, extracted);
		}
		return 0;
}
void init()
{
		pc1 = {
				57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36, 63,
				55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22, 14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4
		};
		pc2 = {
				14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10, 23, 19, 12, 4, 26, 8, 16, 7, 27, 20, 13, 2, 41, 52, 31, 37, 47, 55,
				30, 40, 51, 45, 33, 48, 44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
		};
		IP_t = {
				58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32,
				24, 16, 8, 57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3, 61, 53, 45, 37, 29, 21, 13, 5, 63, 55,
				47, 39, 31, 23, 15, 7
		};
		IP_1 = {
				40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53,
				21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26, 33, 1,
				41, 9, 49, 17, 57, 25
		};
		E_t = {
				32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9, 8, 9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17, 16, 17, 18, 19, 20, 21, 20,
				21, 22, 23, 24, 25, 24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1
		};
		s = {{{ 14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7 },
				{ 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8 },
				{ 4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0 },
				{ 15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13 }},
				{{ 15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10 },
						{ 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5 },
						{ 0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15 },
						{ 13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9 }},
				{{ 10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8 },
						{ 13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1 },
						{ 13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7 },
						{ 1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12 }},
				{{ 7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15 },
						{ 13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9 },
						{ 10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4 },
						{ 3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 }},
				{{ 2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9 },
						{ 14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6 },
						{ 4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14 },
						{ 11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 }},
				{{ 12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11 },
						{ 10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8 },
						{ 9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6 },
						{ 4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 }},
				{{ 4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1 },
						{ 13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6 },
						{ 1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2 },
						{ 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 }},
				{{ 13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7 },
						{ 1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2 },
						{ 7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8 },
						{ 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 }}};;
		P = {
				16, 7, 20, 21, 29, 12, 28, 17, 1, 15, 23, 26, 5, 18, 31, 10, 2, 8, 24, 14, 32, 27, 3, 9, 19, 13, 30, 6, 22, 11,
				4, 25
		};
		P_1 = {
				40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31, 38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53,
				21, 61, 29, 36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27, 34, 2, 42, 10, 50, 18, 58, 26, 33, 1,
				41, 9, 49, 17, 57, 25
		};
}