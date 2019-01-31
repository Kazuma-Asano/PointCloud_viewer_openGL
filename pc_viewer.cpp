#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <cmath>

#include<GL/glew.h>
#include<SDL2/SDL.h>
#include<glm/glm.hpp>
#include<glm/ext.hpp>

// Read file
#include<iostream>
#include<fstream>
#include<string>
#include "picojson.h"

#include <opencv2/opencv.hpp>
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
////////////////////////////////////////////////////////////////////////////////

void saveImage( const unsigned int img_w, const unsigned int img_h, int frame_num ) {
  const unsigned int ch = 3;
  void* dataBuffer = NULL;
  dataBuffer = (GLubyte*)malloc( img_w * img_h *ch);

  glReadBuffer(GL_BACK);
  glReadPixels(
    0, // lower left x is ?
    0, // lower left y is ?
    img_w,
    img_h,
    GL_BGR, // Read color
    GL_UNSIGNED_BYTE, // Read data type
    dataBuffer //bit map pixels pointer
  );

  GLubyte* p = static_cast<GLubyte*>(dataBuffer);
  std::stringstream ss;
  ss << frame_num << ".png";
  std::string filename = ss.str();
  IplImage* outImage = cvCreateImage( cvSize(img_w, img_h), IPL_DEPTH_8U, 3);

  for ( unsigned int j = 0; j < img_h; ++j)
  {
    for ( unsigned int i = 0; i < img_w; ++i)
    {
      outImage->imageData[ (img_h - j - 1 ) * outImage->widthStep + i * 3 + 0 ] = *p;
      outImage->imageData[ (img_h - j - 1 ) * outImage->widthStep + i * 3 + 1 ] = *( p + 1 );
      outImage->imageData[ (img_h - j - 1 ) * outImage->widthStep + i * 3 + 2 ] = *( p + 2 );
      p += 3;
    }
  }
  // cvSaveImage( filename.c_str(), outImage );

}
//////////////////////////////////////////////////////////////////////////////


#undef main
int main() {
  unsigned int width = 1392;
  unsigned int height = 512;

	//SDL初期設定
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); //Compatibiltyプロファイル指定にしてClassicな機能を使う
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // SDL_CreateWindow(const char* title, int screen_x, int screen_y, int w, int h, Uint32 flags)
  SDL_Window *window = SDL_CreateWindow("FixedFunctionShaderTest", width/2, height/2, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
  SDL_GLContext context = SDL_GL_CreateContext(window);

	//glew初期化＠GLコンテキスト作った後
	glewInit();

	//OpenGLでエラーが起きた場合のコールバック
	glDebugMessageCallback(mygldbgcallback, NULL);

	//描画領域の大きさ
	int dwindowWidth, dwindowHeight;
	SDL_GL_GetDrawableSize(window, &dwindowWidth, &dwindowHeight);

	//点群を読み込む
	std::fstream ifspc("./kitti_20110926_0029/depthmaps/pointCloud.ply"); //点群ファイル読み込み
  // std::fstream ifspc("./kitti2/depthmaps/pointCloud.ply"); //点群ファイル読み込み
	std::string str;
	std::vector<float> pc_position_buf;
	std::vector<float> pc_color_buf;
	int count = 0;
	bool is_float;

	if(ifspc.fail()) {
		std::cout << "Failed" << std::endl;
		return -1;
	}

  // 点群読み込み
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
			pc_position_buf.push_back(y); //z
			pc_position_buf.push_back(z); //y
			//色は0~1で指定
			pc_color_buf.push_back(r); //r
			pc_color_buf.push_back(g); //g
			pc_color_buf.push_back(b); //b
		}
	}

  // // カメラ軌道確認用 四角錐
  std::vector<float> cube0_pos_buf;
  std::vector<float> cube0_col_buf;

  {
    float tvbuf[]={
      0,0,0, //頂点
      -100,100,100,
      0,0,0,
      -100,-100,100,
      0,0,0,
      100,-100,100,
      0,0,0,
      100,100,100,
      -100,100,100,
      -100,-100,100,
      -100,-100,100,
      100,-100,100,
      100,-100,100,
      100,100,100,
      100,100,100,
      -100,100,100
    };
    cube0_pos_buf.clear(); cube0_pos_buf.resize(sizeof(tvbuf)/sizeof(float));
    memcpy(&cube0_pos_buf[0],tvbuf,sizeof(tvbuf));
  }

  // color
  {
    float tcbuf[]={
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1,
      1,1,1
    };
    cube0_col_buf.clear(); cube0_col_buf.resize(sizeof(tcbuf)/sizeof(float));
    memcpy(&cube0_col_buf[0],tcbuf,sizeof(tcbuf));
  }

  // VAO
  GLuint cam_vao;
	glGenVertexArrays(1, &cam_vao);
	glBindVertexArray(cam_vao);
	GLuint cam_vbos[2];
	glGenBuffers(2, cam_vbos);
	glBindBuffer(GL_ARRAY_BUFFER, cam_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube0_pos_buf.size(), &cube0_pos_buf[0], GL_STATIC_DRAW); //データ転送
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 3, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, cam_vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube0_col_buf.size(), &cube0_col_buf[0], GL_STATIC_DRAW); //データ転送
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, sizeof(float) * 3, (GLvoid*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pc_vbos[2]); //インデックス配列付ける場合
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pc_indices_buf.size(), &pc_indices_buf[0], GL_STATIC_DRAW); //データ転送
	glBindVertexArray(0);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

  // transration読み込み
  std::vector<glm::vec3> translist;
  std::vector<glm::vec3> rotatelist;
  std::vector<glm::mat4> extrinsics;
  std::vector<glm::mat4> mat_model_cams;
  {
    std::fstream ifsjson("./kitti_20110926_0029/reconstruction.json"); //ファイル読み込み
    // std::fstream ifsjson("./kitti_20110926_0015/reconstruction.json");
    if(ifsjson.fail()) {
      std::cout << "reconstruction　file read Failed" << std::endl;
      return -1;
    }
    // Picojsonへ読み込む
    picojson::value val;
    ifsjson >> val;
    // fs変数はもう使わないので閉鎖
    ifsjson.close();

    picojson::object& obj = val.get<picojson::array>()[0].get<picojson::object>();
    picojson::object& shots = obj["shots"].get<picojson::object>();
    for (picojson::object::iterator i = shots.begin(); i != shots.end(); ++i)
    {
      std::cout << i -> first << std::endl;

      picojson::array& trans = i->second.get<picojson::object>()["translation"].get<picojson::array>();
      translist.push_back(glm::vec3((float)trans[0].get<double>(), (float)trans[1].get<double>(), (float)trans[2].get<double>()));
      std::cout << "x_t:" << translist.back()[0] << std::endl;
      std::cout << "y_t:" << translist.back()[1] << std::endl;
      std::cout << "z_t:" << translist.back()[2] << std::endl;
      //std::cout << "x_t:" << trans[0].get<double>() << std::endl;
      //std::cout << "y_t:" << trans[1].get<double>() << std::endl;
      //std::cout << "z_t:" << trans[2].get<double>() << std::endl;

      picojson::array& rotas = i->second.get<picojson::object>()["rotation"].get<picojson::array>();
      rotatelist.push_back(glm::vec3((float)rotas[0].get<double>(), (float)rotas[1].get<double>(), (float)rotas[2].get<double>()));
      std::cout << "x_r:" << rotatelist.back()[0] << std::endl;
      std::cout << "y_r:" << rotatelist.back()[1] << std::endl;
      std::cout << "z_r:" << rotatelist.back()[2] << std::endl;
      // cv::Mat rvec(3,1,CV_32FC1,glm::value_ptr(rotatelist.back()));
      // cv::Mat rmat(3,3,CV_32FC1);
      // cv::Rodrigues(rvec,rmat);
      // cv::transpose(rmat,rmat);
      // glm::mat3 t=glm::make_mat3((float*)rmat.data);
      //std::cout << "x_r:" << rotas[0].get<double>() << std::endl;
      //std::cout << "y_r:" << rotas[1].get<double>() << std::endl;
      //std::cout << "z_r:" << rotas[2].get<double>() << std::endl;

      float length = glm::length(rotatelist.back());
      //glm::mat4 t = glm::rotate(glm::degrees(length), glm::normalize(rotatelist.back()));
      extrinsics.push_back(
       glm::translate(glm::mat4(1.0f), translist.back()) * glm::rotate(length, glm::normalize(rotatelist.back()))
      );

      // cv rodorigues
      // extrinsics.push_back(
      //   glm::translate(glm::mat4(1.0f), translist.back()) * glm::mat4(t)
      // );
        std::cout << "extrinsics:" << glm::to_string(extrinsics.back()) << std::endl;
      // glm::inverse(extrinsics.back())
      //
      // glm::vec3 up = glm::vec3(-glm::row(extrinsics.back(), 1));
      // glm::vec3 front = glm::vec3(-glm::row(extrinsics.back(), 2));
      // glm::vec3 eye = glm::inverse(glm::mat3(extrinsics.back())) * glm::vec3(-glm::column(extrinsics.back(), 3));
      //
      // glm::vec3 lookat = eye - front;
      // glm::mat4 mat_model_cam = glm::inverse(glm::lookAt(eye, lookat, up));
      // glm::mat4 mat_model_cam = glm::lookAt(eye, lookat, up);
      // mat_model_cams.push_back(glm::inverse(extrinsics.back()));
    }
  }

  // picojosn::array trans=shots["0000000237.png"].get<picojson::object>()["translation"].get<picojosn::array>();
  // trans[0].get<double>();
  // trans[1].get<double>();
  // trans[2].get<double>();
  // std::vector<glm::mat4> extrinsics;


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
  // 固定カメラ
	// glm::vec3 camPos(0, 0, 1000);
	// glm::vec3 gazePos(0.0, 0.0, 0.0);
	// glm::vec3 upDir(0.0, 1.0, 0.0);

	// glm::mat4 mat_proj = glm::perspective(45.0, (double)dwindowWidth / dwindowHeight, 0.01, 100.0);
	// glm::mat4 mat_fixedview = glm::lookAt(camPos, gazePos, upDir); // mat_view(LookAt)をextirc paraに変える
	// glm::mat4 mat_model(1);
  // glm::mat4 mat_vm = mat_view * mat_model;　

  //glmが使えるはず 64 = 画角
  glm::mat4 mat_proj = glm::perspective(64.0, (double)dwindowWidth/dwindowHeight, 0.01, 2000.0);
  // auto &cc = extrinsics[1];
  glm::mat4 mat_view = extrinsics[0];
	glm::mat4 mat_model(1.0f);
  glm::mat4 mat_vm=mat_view * mat_model;

	//行列設定
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(mat_proj)); //指定したメモリで行列を埋める．掛け算ではない

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(mat_vm)); //指定したメモリで行列を埋める．掛け算ではない

	//その他設定
	glViewport(0, 0, dwindowWidth, dwindowHeight);
	glClearColor(0.2, 0.2, 0.2, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE); glPointSize(2); //点のサイズの設定．なぜか1だと表示されないので2にしてみた．小さすぎて映らない？

	SDL_GL_SetSwapInterval(1); //VSYNC使う


	//メインループ
	bool loopFlg = true;
  int fcount=0;
	while (loopFlg) {
		//初期化
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 点群を描写するフレーム
    int frame_num = fcount % extrinsics.size();

    // カメラの撮影を点群内で再現
    // 行列適用
    mat_view = extrinsics[frame_num];
    auto x_rotate = glm::rotate(glm::mat4(), (float)glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f) ); // カメラをx軸に180°回転
    mat_view =  x_rotate * mat_view;
    auto y_rotate = glm::rotate(glm::mat4(), (float)glm::radians(120.0f), glm::vec3(0.0f, 1.0f, 0.0f) ); // カメラをy軸に60°回転
    mat_view =  y_rotate * mat_view;
    glm::mat4 mat_vm = mat_view * mat_model;


    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(mat_vm));

    //点群を描画
		glBindVertexArray(pc_vao);
		glDrawArrays(GL_POINTS, 0, pc_position_buf.size() / 3); //描画指示
		glBindVertexArray(0);

    // camera 軌道
    // glm::mat4 mat_camera_model = mat_model_cams[frame_num];
    // // glm::mat4 mat_camera_model = translist[frame_num];
    // mat_vm = mat_fixedview * mat_camera_model;
    //
    // glMatrixMode(GL_MODELVIEW);
    // glLoadMatrixf(glm::value_ptr(mat_vm));
    //
    // glBindVertexArray(cam_vao);
		// glDrawArrays(GL_LINES, 0, cube0_pos_buf.size() / 3); //描画指示
		// glBindVertexArray(0);


		//描画完了
		glFlush();

		//描画をウィンドウに反映
		SDL_GL_SwapWindow(window);
    SDL_Delay(100);


    //描画ウィンドウを撮影
    saveImage(width, height, frame_num);

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
