
# include <Siv3D.hpp> // OpenSiv3D v0.4.3

struct AfinParameter3D {
	double a;
	double b;
	double c;
	double d;
	double e;
	double f;
	double g;
	double h;
	double i;
	double j;
	double k;
	double l;
	double m;
	double n;
	double o;
	double p;
};
struct Angle {
	double w;
	double h;
};
struct Object {
	Angle angle;
	Vec3 pos;
};
struct _Triangle3D {
	Triangle3D points;
	Color color;
};
struct _Triangle {
	Triangle points;
	Color color;
};
struct Model {
	Array<_Triangle3D> shape;
	Object object;
	Vec3 corePos;
	int hp;
};
AfinParameter3D viewingPiperine;


// 共通
Vec3 changePos3D(Vec3 p, AfinParameter3D afin) {
	Vec3 res;
	res.x = afin.a * p.x + afin.b * p.y + afin.c * p.z + afin.d;
	res.y = afin.e * p.x + afin.f * p.y + afin.g * p.z + afin.h;
	res.z = afin.i * p.x + afin.j * p.y + afin.k * p.z + afin.l;
	return res;
}
_Triangle3D transFormTriangle3D(_Triangle3D t, AfinParameter3D afin) {
	t.points.p0 = changePos3D(t.points.p0, afin);
	t.points.p1 = changePos3D(t.points.p1, afin);
	t.points.p2 = changePos3D(t.points.p2, afin);
	return t;
}
Array<_Triangle3D> transFormModel(Array<_Triangle3D> triangles, AfinParameter3D afin) {
	return triangles.map([afin](_Triangle3D t) { return transFormTriangle3D(t, afin); });
}
AfinParameter3D combineAfin(AfinParameter3D x, AfinParameter3D y) {
	AfinParameter3D res;
	res.a = x.a * y.a + x.e * y.b + x.i * y.c + x.m * y.d;
	res.b = x.b * y.a + x.f * y.b + x.j * y.c + x.n * y.d;
	res.c = x.c * y.a + x.g * y.b + x.k * y.c + x.o * y.d;
	res.d = x.d * y.a + x.h * y.b + x.l * y.c + x.p * y.d;
	res.e = x.a * y.e + x.e * y.f + x.i * y.g + x.m * y.h;
	res.f = x.b * y.e + x.f * y.f + x.j * y.g + x.n * y.h;
	res.g = x.c * y.e + x.g * y.f + x.k * y.g + x.o * y.h;
	res.h = x.d * y.e + x.h * y.f + x.l * y.g + x.p * y.h;
	res.i = x.a * y.i + x.e * y.j + x.i * y.k + x.m * y.l;
	res.j = x.b * y.i + x.f * y.j + x.j * y.k + x.n * y.l;
	res.k = x.c * y.i + x.g * y.j + x.k * y.k + x.o * y.l;
	res.l = x.d * y.i + x.h * y.j + x.l * y.k + x.p * y.l;
	res.m = x.a * y.m + x.e * y.n + x.i * y.o + x.m * y.p;
	res.n = x.b * y.m + x.f * y.n + x.j * y.o + x.n * y.p;
	res.o = x.c * y.m + x.g * y.n + x.k * y.o + x.o * y.p;
	res.p = x.d * y.m + x.h * y.n + x.l * y.o + x.p * y.p;
	return res;
}

//-----
//ポリゴンの表裏判定
//-------
Vec3 cross_product(const Vec3 vl, const Vec3 vr)
{
	Vec3 ret;
	ret.x = vl.y * vr.z - vl.z * vr.y;
	ret.y = vl.z * vr.x - vl.x * vr.z;
	ret.z = vl.x * vr.y - vl.y * vr.x;

	return ret;
}

//ベクトル内積
double dot_product(const Vec3 vl, const Vec3 vr) {
	return vl.x * vr.x + vl.y * vr.y + vl.z * vr.z;
}

// ベクトルvに対してポリゴンが表裏どちらを向くかを求める
// 戻り値    0:表    1:裏    -1:エラー
int polygon_side_chk(Triangle3D t, Vec3 v) {

	//ABCが三角形かどうか。ベクトルvが0でないかの判定は省略します
	Vec3 A = t.p0;
	Vec3 B = t.p1;
	Vec3 C = t.p2;
	//AB BCベクトル
	Vec3 AB;
	Vec3 BC;

	AB.x = B.x - A.x;
	AB.y = B.y - A.y;
	AB.z = B.z - A.z;

	BC.x = C.x - A.x;
	BC.y = C.y - A.y;
	BC.z = C.z - A.z;

	//AB BCの外積
	Vec3 c = cross_product(AB, BC);
	double dist = t.p0.z + t.p1.z + t.p2.z;
	if (dist < 3) {
		return 0;
	}
	//ベクトルvと内積。順、逆方向かどうか調べる
	double d = dot_product(v, c);
	
	if (d < 0.0) {
		return 1;    //ポリゴンはベクトルvから見て表側
	}
	return 0;
}
bool isFartherTriangle(_Triangle3D t, _Triangle3D a) {
	double targetDist = t.points.p0.z + t.points.p1.z + t.points.p2.z;
	double dist = a.points.p0.z + a.points.p1.z + a.points.p2.z;
	return targetDist > dist;
}
Array<_Triangle3D> sortTriangle3D(Array<_Triangle3D> triangles) {//奥行ソート
	for (int i = 0; i < triangles.size(); i++) {
		for (int j = i; j < triangles.size(); j++) {
			if (isFartherTriangle(triangles[i], triangles[j])) {
				_Triangle3D tmp = triangles[i];
				triangles[i] = triangles[j];
				triangles[j] = tmp;
			}
		}
	}
	return triangles;
}
// 投影変換
Vec2 toVec2(Vec3 pos) {
	return Vec2{ pos.x,pos.y };
	//return Vec2{ pos.x/pos.z*500,pos.y/pos.z*500 };//投視投影　現時点だと歪んで見える
}
_Triangle renderTriangle(_Triangle3D t) {
	_Triangle result;
	result.points.p0 = toVec2(t.points.p0);
	result.points.p1 = toVec2(t.points.p1);
	result.points.p2 = toVec2(t.points.p2);
	result.color = t.color;

	return result;
}
Array<_Triangle> renderModel(Array<_Triangle3D> triangles) {
	_Triangle n = {};
	triangles = sortTriangle3D(triangles);
	
	return triangles.map([n](_Triangle3D t) { return polygon_side_chk(t.points, Vec3{ 0,0,1 }) ? renderTriangle(t) : n; });
}
Array<_Triangle> render(Array<Model> models) {
	Array<_Triangle> res = {};
	for (int i = 0; i < models.size(); i++) {
		 Array<_Triangle> toAdd = renderModel(models[i].shape);
		 for (int i = 0; i < toAdd.size(); i++) {
			 res << toAdd[i];
		 }
	}
	return res;
}
//ビューポート変換
Vec2 moveCenterPos(Vec2 p) {
	return p + Scene::Center();
}
_Triangle moveCenterTriangle(_Triangle t) {
	t.points.p0 = moveCenterPos(t.points.p0);
	t.points.p1 = moveCenterPos(t.points.p1);
	t.points.p2 = moveCenterPos(t.points.p2);
	return t;
}
Array<_Triangle> moveCenterModel(Array<_Triangle> triangles) {
	return triangles.map([](_Triangle t) { return moveCenterTriangle(t); });
}

//モデリング変換
Array<_Triangle3D> toWorldModel(Array<_Triangle3D> triangles, Object object) {
	AfinParameter3D afin1, afin2,afin3;
	double w = object.angle.w / 50;
	double h = object.angle.h / 50;
	afin1 = { cos(w),0,-sin(w),0,0,1,0,0,sin(w),0,cos(w),0 };
	afin2 = { 1,0,0,0,
		0,cos(h),sin(h),0,
		0,-sin(h),cos(h),0 };
	triangles = transFormModel(triangles, combineAfin(afin2, afin1));
	viewingPiperine = combineAfin(afin2, afin1);
	Vec3 p = object.pos;
	afin3 = { 1,0,0,-p.x,0,1,0,-p.y,0,0,1,-p.z };
	triangles = transFormModel(triangles, afin3);
	return triangles;
}
Array<Model> toWorld(Array<Model> models) {
	for (int i = 0; i < models.size(); i++) {
		models[i].shape = toWorldModel(models[i].shape, models[i].object);
	}
	return models;
}
// 視野変換

Array<_Triangle3D> conversionFieldModel(Array<_Triangle3D> triangles, Object camera) {
	AfinParameter3D afin1, afin2,afin3,afin4;
	
	Vec3 p = camera.pos;
	afin3 = { 1,0,0,-p.x,0,1,0,-p.y,0,0,1,-p.z };
	triangles = transFormModel(triangles, afin3);
	double w = camera.angle.w / 50;
	double h = camera.angle.h / 50;
	afin1 = { cos(w),0,-sin(w),0,0,1,0,0,sin(w),0,cos(w),0 };
	afin2 = { 1,0,0,0,0,cos(h),sin(h),0,0,-sin(h),cos(h),0 };
	triangles = transFormModel(triangles, combineAfin(afin2, afin1));
	
	return triangles;
}
Array<Model> conversionField(Array<Model> models,Object camera) {
	for (int i = 0; i < models.size(); i++) {
		models[i].shape = toWorldModel(models[i].shape, camera);
	}
	return models;
}


void Main()
{
	// 背景を黒にする
	Scene::SetBackground(Palette::Black);

	// 大きさ 60 のフォントを用意
	const Font font(60);

	//モデリング
	Array<Vec3> samplePoints = {
		//{0,0,0},{200,0,0},{200,200,0},{0,200,0},
		{-20,20,-20},{0,200,0},{200,0,0},
		{0,0,200},{300,300,300},{0,0,100},{0,100,0},
		{100,80,100},{0,80,100}
	};
	Array<_Triangle3D> samplePolygons = {
		{Triangle3D{ samplePoints[0], samplePoints[2], samplePoints[1] },Color{255,0,0}},
		{Triangle3D{ samplePoints[0], samplePoints[1], samplePoints[3] },Color{100,255,0}},
		{Triangle3D{ samplePoints[0], samplePoints[3], samplePoints[2] },Color{100,0,0}},
		{Triangle3D{ samplePoints[4], samplePoints[1], samplePoints[2] },Color{100,0,255}},
		{Triangle3D{ samplePoints[4], samplePoints[3], samplePoints[1] },Color{0,0,100}},
		{Triangle3D{ samplePoints[4], samplePoints[2], samplePoints[3] },Color{100,100,100}},
	};
	Array<Vec3> cubePoints = {
	{-100,-100,-100},{100,-100,-100},{100,-100,100},{-100,-100,100},
	{-100,100,-100},{100,100,-100},{100,100,100},{-100,100,100}
	};
	Array<_Triangle3D> cubePolygons = {
	{Triangle3D{ cubePoints[0], cubePoints[3], cubePoints[1] },Color{255,0,0}},
	{Triangle3D{ cubePoints[1], cubePoints[3], cubePoints[2] },Color{255,0,0}},
	{Triangle3D{ cubePoints[4], cubePoints[5], cubePoints[7] },Color{0,0,255}},
	{Triangle3D{ cubePoints[5], cubePoints[6], cubePoints[7] },Color{0,0,255}},
	{Triangle3D{ cubePoints[0], cubePoints[5], cubePoints[4] },Color{0,255,0,80}},
	{Triangle3D{ cubePoints[1], cubePoints[5], cubePoints[0] },Color{0,255,00,80}},
	{Triangle3D{ cubePoints[0], cubePoints[4], cubePoints[7] },Color{0,255,255,80}},
	{Triangle3D{ cubePoints[3], cubePoints[0], cubePoints[7] },Color{0,255,255,80}},

	{Triangle3D{ cubePoints[2], cubePoints[7], cubePoints[6] },Color{255,255,0}},
	{Triangle3D{ cubePoints[3], cubePoints[7], cubePoints[2] },Color{255,255,0}},

	{Triangle3D{ cubePoints[2], cubePoints[6], cubePoints[5] },Color{255,0,255}},
	{Triangle3D{ cubePoints[1], cubePoints[2], cubePoints[5] },Color{255,0,255}},

	};
	Object ex1 = { Angle{0,0},Vec3{-150,0,-500} };
	Object sample = { Angle{0,-12},Vec3{0,-40,-800} };
	Array<Model> models = {
		{samplePolygons,ex1,{0,0,0},100},
		{cubePolygons,sample,{0,0,0},100},
	};
	//samplePolygons = cubePolygons;
	//モデリング変換
	Array<_Triangle3D> sample_W = toWorldModel(samplePolygons, sample);
	Array<Model> models_W = toWorld(models);
	for (int i = 0; i < cubePolygons.size(); i++) {
		samplePolygons << cubePolygons[i];
	}
	Object camera = { Angle{640,-10},Vec3{0,0,0} };
	Grid<int32> fieldState(32, 32, 0);//３次元配列　値で１列を管理

	while (System::Update())
	{
		const double delta = 200 * Scene::DeltaTime();

		// 上下左右キーで移動
		if (KeyLeft.pressed())
		{
			models[1].object.pos.x += delta;
		}

		if (KeyRight.pressed())
		{
			models[1].object.pos.x -= delta;
		}

		if (KeyUp.pressed())
		{
			models[1].object.pos.y -= delta;
		}

		if (KeyDown.pressed())
		{
			models[1].object.pos.y += delta;
		}

		ClearPrint();
		//モデリング変換
		models[1].object.angle.w += 1.7;
		//sample.angle.h -=1.6;
		const double hue = Scene::Time() * 60.0;
		models[0].shape[4].color = HSV(hue, 0.6, 1.0);
		sample_W = toWorldModel(samplePolygons, sample);
		models_W = toWorld(models);
		//視野変換
		models[0].object.angle.w = Cursor::Pos().x - Scene::Center().x;
		models[0].object.angle.h = Cursor::Pos().y - Scene::Center().y;
		Array<_Triangle3D> sample_W_camera = conversionFieldModel(sample_W, camera);
		Array<Model> models_W_camera = conversionField(models_W, camera);

		sample_W_camera[0].color = Color(Palette::Red);

		// 投影変換

		//Array<_Triangle> t = renderModel(sample_W_camera);
		Array<_Triangle> t = render(models_W_camera);
		// ビューポート変換
		t = moveCenterModel(t);
		Print << t[7].color;
		Print << t[0].color;
		//描画
		t.map([](_Triangle t) {t.points.draw(t.color);  return 0; });
		
		//デバッグ
		Print << Cursor::Pos(); // 現在のマウスカーソル座標を表示
		Print << camera.angle.w;
		Print << sample.angle.h;
		Print << camera.pos;
		//Line(600, 500, 600 + cos(camera.angle.w/50)*20,500 + sin(camera.angle.w/50)*20).draw(3, Palette::Red);
	}
}

//参考　http://www.sousakuba.com/Programming/gs_polygon_inside_outside.html ポリゴン表裏判定

//
// = アドバイス =
// Debug ビルドではプログラムの最適化がオフになります。
// 実行速度が遅いと感じた場合は Release ビルドを試しましょう。
// アプリをリリースするときにも、Release ビルドにするのを忘れないように！
//
// 思ったように動作しない場合は「デバッグの開始」でプログラムを実行すると、
// 出力ウィンドウに詳細なログが表示されるので、エラーの原因を見つけやすくなります。
//
// = お役立ちリンク =
//
// OpenSiv3D リファレンス
// https://siv3d.github.io/ja-jp/
//
// チュートリアル
// https://siv3d.github.io/ja-jp/tutorial/basic/
//
// よくある間違い
// https://siv3d.github.io/ja-jp/articles/mistakes/
//
// サポートについて
// https://siv3d.github.io/ja-jp/support/support/
//
// Siv3D ユーザコミュニティ Slack への参加
// https://siv3d.github.io/ja-jp/community/community/
//
// 新機能の提案やバグの報告
// https://github.com/Siv3D/OpenSiv3D/issues
//
