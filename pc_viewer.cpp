#include<stdio.h>
#include<stdlib.h>
#include<vector>

#include<GL/glew.h>
#include<SDL2/SDL.h>
#include<glm/glm.hpp>
#include<glm/ext.hpp>

// Read file
#include<iostream>
#include<fstream>
#include<string>
#include "picojson.h"
using namespace std;


////////////////////////////////////////////////////////////////////////////////
// キャスト変換
float stofTryCatch(const std::string& a, bool* is_float) {
  float result = 0.0f;
  try {
    result = std::stof(a);
  } catch (const std::invalid_argument&) {
    // std::cout << "Error: The string '" << a << "' is not float." << std::endl;
    *is_float = false;
    return result;
  } catch (const std::out_of_range&) {
    // std::cout << "Error: The string '" << a << "' is float but out of range." << std::endl;
    *is_float = false;
    return result;
  }
  *is_float = true;
  // std::cout << "'" << a << "' -> " << result << std::endl;
  return result;
}
////////////////////////////////////////////////////////////////////////////////

//Split関数
vector<string> split(const string& src, const char* delim = " ") { //スペース区切り
    vector<string> vec;
    string::size_type len = src.length();

    for (string::size_type i = 0, n; i < len; i = n + 1) {
        n = src.find_first_of(delim, i);
        if (n == string::npos) {
            n = len;
        }
        vec.push_back(src.substr(i, n - i));
    }

    return vec;
}
////////////////////////////////////////////////////////////////////////////////

void GLAPIENTRY mygldbgcallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	printf("[GLDBG]: %s\n", message);
	if (type == GL_DEBUG_TYPE_ERROR) {
		abort();
	}
}

#undef main
int main() {
	//SDL初期設定
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); //Compatibiltyプロファイル指定にしてClassicな機能を使う
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Window *window = SDL_CreateWindow("FixedFunctionShaderTest", 50, 50, 1080, 512, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	//glew初期化＠GLコンテキスト作った後
	glewInit();

	//OpenGLでエラーが起きた場合のコールバック
	glDebugMessageCallback(mygldbgcallback, NULL);

	//描画領域の大きさ
	int dwindowWidth, dwindowHeight;
	SDL_GL_GetDrawableSize(window, &dwindowWidth, &dwindowHeight);

	//点群を読み込む
	std::fstream ifspc("./kitti2/depthmaps/pointCloud.ply"); //点群ファイル読み込み
	std::string str;
	std::vector<float> pc_position_buf;
	std::vector<float> pc_color_buf;
	int count = 0;
	bool is_float;

	if(ifspc.fail()) {
		std::cout << "Failed" << std::endl;
		return -1;
	}

	while( (getline(ifspc, str)) ) {
		count++;
		if (count  > 14) { //点群読み込み開始位置
			vector<string> vec = split(str);

			float x = stofTryCatch(vec[0], &is_float);
			float y = stofTryCatch(vec[1], &is_float);
			float z = stofTryCatch(vec[2], &is_float);
			// float nx = stofTryCatch(vec[3], &is_float);
			// float ny = stofTryCatch(vec[4], &is_float);
			// float nz = stofTryCatch(vec[5], &is_float);
			float r = stofTryCatch(vec[6], &is_float) / 255;
			float g = stofTryCatch(vec[7], &is_float) / 255;
			float b = stofTryCatch(vec[8], &is_float) / 255;

			pc_position_buf.push_back(x); //x
			pc_position_buf.push_back(z); //y
			pc_position_buf.push_back(y); //z
			//色は0~1で指定
			pc_color_buf.push_back(r); //r
			pc_color_buf.push_back(g); //g
			pc_color_buf.push_back(b); //b
		}
	}

	//点群の位置，色情報（別々にする場合）
	//※位置，色を一つにまとめる方法もある．一応そちらのほうが効率的だが誤差レベル？
	// std::vector<float> pc_position_buf;
	// std::vector<float> pc_color_buf; //unsigned charでも可能．その場合はglColorPointerのTYPEをGL_UNSIGNED_BYTEにする．
	//サンプルで点群作ります．実際には読み込んだ点群で埋めるべき．
	// for (int r = 0; r < 10; ++r) {
	// 	for (int c = 0; c < 10; ++c) {
	// 		float x = c / 10.0, y = 0, z = r / 10.0;
	// 		pc_position_buf.push_back(x); //x
	// 		pc_position_buf.push_back(z); //y
	// 		pc_position_buf.push_back(y); //z
	// 		//色は0~1で指定
	// 		float r = c / 10.0, g = 0, b = 1.0 - r;
	// 		pc_color_buf.push_back(r); //r
	// 		pc_color_buf.push_back(g); //g
	// 		pc_color_buf.push_back(b); //b
	// 		//pc_color_buf.push_back(1.0); //a //4チャンネルにしてアルファブレンドも設定によっては可能．
	// 	}
	// }
	//インデックス配列
	//インデックス配列で指定することで，実際に描画する点をidで選んでその順に描画できる．3Dモデルのように同じ座標の点を何度も指定する場合に省メモリになる．
	//その場合はglDrawElementsで描画する．
	//std::vector<unsigned int> pc_indices_buf;

	//点群のデータを示すVAOの作成
	GLuint pc_vao;
	glGenVertexArrays(1, &pc_vao);
	glBindVertexArray(pc_vao);
	GLuint pc_vbos[2];
	glGenBuffers(2, pc_vbos);
	glBindBuffer(GL_ARRAY_BUFFER, pc_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pc_position_buf.size(), &pc_position_buf[0], GL_STATIC_DRAW); //データ転送
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 3, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, pc_vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pc_color_buf.size(), &pc_color_buf[0], GL_STATIC_DRAW); //データ転送
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, sizeof(float) * 3, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pc_vbos[2]); //インデックス配列付ける場合
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pc_indices_buf.size(), &pc_indices_buf[0], GL_STATIC_DRAW); //データ転送
	glBindVertexArray(0);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	//固定機能シェーダを使う場合は行列系のGL関数を使って設定
	// glm::vec3 camPos(1.0, 1.0, 1.0);
	// glm::vec3 gazePos(0.0, 0.0, 0.0);
	// glm::vec3 upDir(0.0, 1.0, 0.0);
	//めんどいのでglm使う
	// glm::mat4 mat_proj = glm::perspective(45.0, (double)dwindowWidth / dwindowHeight, 0.01, 100.0);
	// glm::mat4 mat_view = glm::lookAt(camPos, gazePos, upDir);　// mat_view(LookAt)をextirc paraに変える
	// glm::mat4 mat_model(1);
  // glm::mat4 mat_vm = mat_view * mat_model;　

  //////////////////////////////////////////////////////////////////////////////
  //実際にはOpenSfMから拾った行列データで適当に回転の計算をかけてそのまま突っ込めばいい
  std::fstream ifsjson("./kitti2/trajectory.json"); //ファイル読み込み
  if(ifsjson.fail()) {
    std::cout << "trajectory Failed" << std::endl;
    return -1;
  }
  // Picojsonへ読み込む
  picojson::value val;
  ifsjson >> val;
  // fs変数はもう使わないので閉鎖
  ifsjson.close();

  // extrinsics paramaterを取得
  picojson::object& obj = val.get<picojson::object>();
  picojson::array& ary = obj["extrinsic"].get<picojson::array>();
  std::vector<glm::mat4> extrinsics;

  for (auto& e : ary) {  // vectorをrange-based-forでまわしている。
    auto &ea = e.get<picojson::array>();
    int i=0;
    extrinsics.push_back(glm::mat4(1.0)); //ひとまず単位行列を後ろに挿入
    for (auto& c : ea) {
      glm::value_ptr(extrinsics.back())[i++]=c.get<double>();
    }
  }
  //YとZをスワップ
  //参照渡しで2行目を反転
  glm::vec4 temp;
  for (auto &cc : extrinsics){
    for(int i = 0; i < 3; i++) {
      cc[i][1] = cc[i][1] * -1;
    }
    temp = cc[1];
    cc[1] = cc[2]; // y -> z
    cc[2] = temp; // z -> y
    // std::cout << glm::to_string(cc[1]) << std::endl;
  }
  //glmが使えるはず
  glm::mat4 mat_proj = glm::perspective(45.0, (double)dwindowWidth / dwindowHeight, 0.01, 200.0);
  // auto &cc = extrinsics[1];
  glm::mat4 mat_view = extrinsics[0];
	glm::mat4 mat_model(1);
  glm::mat4 mat_vm=mat_view*mat_model;

	//行列設定
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(mat_proj)); //指定したメモリで行列を埋める．掛け算ではない

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(mat_vm)); //指定したメモリで行列を埋める．掛け算ではない

	//その他設定
	glViewport(0, 0, dwindowWidth, dwindowHeight);
	glClearColor(0, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE); glPointSize(2); //点のサイズの設定．なぜか1だと表示されないので2にしてみた．小さすぎて映らない？

	SDL_GL_SetSwapInterval(1); //VSYNC使う

	//メインループ
	bool loopFlg = true;
  int fcount=0;
	while (loopFlg) {
		//初期化
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//行列適用
    mat_view = glm::inverse(extrinsics[fcount%extrinsics.size()]);
		glm::mat4 mat_vm = mat_view * mat_model;
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(mat_vm));

		//点群を描画
		glBindVertexArray(pc_vao);
		glDrawArrays(GL_POINTS, 0, pc_position_buf.size() / 3); //描画指示
		glBindVertexArray(0);

		//描画完了
		glFlush();

		//描画をウィンドウに反映
		SDL_GL_SwapWindow(window);

		//イベント処理
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					loopFlg = false;
					break;
				}
			}
		}

    fcount++;
	}


	return 0;
}
