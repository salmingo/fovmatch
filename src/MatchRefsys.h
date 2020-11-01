/**
 * @class MatchRefsys 参考系匹配
 * @version 0.1
 * @date Sep 2020
 * @note
 * 函数调用流程:
 * - SetGuessScale
 * - BeginImportImageObject
 * - ImportImageObject
 * - CompleteImportImageObject
 * - BeginImportWcsObject
 * - ImportWcsObject
 * - CompleteImportWcsObject
 * - DoMatch
 */

#ifndef MATCHREFSYS_H_
#define MATCHREFSYS_H_

#include <vector>

class MatchRefsys {
public:
	MatchRefsys();
	virtual ~MatchRefsys();

public:
	/* 数据类型 */
	struct refcenter {
		double x, y;
	};

	struct object_common {
		double x, y;	//< 坐标. 图像系: 像素; 世界系投影: 弧度
		short brightness;	// 整数型星等
	};

	struct object_image : public object_common {
	};
	using ObjImgVec = std::vector<object_image>;

	struct object_wcs  : public object_common {
		double l, b;	// 世界坐标, 弧度
	};
	using ObjWcsVec = std::vector<object_wcs>;

	/*!
	 * @brief 匹配元素的归算结果
	 */
	struct wedge_item {
		int id;
		double incl;	//< 相对定向指向的夹角, 角度
		double lnormal;	//< 归一化距离
	};
	using WedgeItemVec = std::vector<wedge_item>;

	/*!
	 * @struct 匹配单元
	 */
	struct wedge_shape {
		int idCenter;	//< 中心ID
		int idOrient;	//< 朝向ID
		double incl;	//< 倾角: 中心-朝向
		double len;		//< 距离: 中心-朝向
		WedgeItemVec items;	//< 楔子内其它元素的归算结果

	public:
		void reset() {
			items.clear();
		}
	};
	using WedgeShapeVec = std::vector<wedge_shape>;

	/*!
	 * @struct hit_point 命中点
	 * @member id  世界系ID
	 * @member hit 命中次数
	 */
	struct hit_point {
		int id;
		int hit;

	public:
		hit_point() {
			id = hit = 0;
		}

		hit_point(int _id) {
			id  = _id;
			hit = 1;
		}

		void inc() {
			++hit;
		}
	};
	using HitPtVec = std::vector<hit_point>;

	/*!
	 * @struct matched_point 点匹配对应关系
	 */
	struct matched_point {
		HitPtVec idPeer;

	public:
		virtual ~matched_point() {
			idPeer.clear();
		}

		void reset() {
			idPeer.clear();
		}

		void add_point(int id) {
			HitPtVec::iterator it;
			for (it = idPeer.begin(); it != idPeer.end() && it->id != id; ++it);
			if (it != idPeer.end()) it->inc();
			else {
				hit_point pt(id);
				idPeer.push_back(pt);
			}
		}

		/*!
		 * @brief 提取命中率最高的WCS目标ID
		 * @param ratio 命中率最高与次高的比值
		 * @return
		 * 命中率最高的ID
		 */
		int get_maxhit(double &ratio) {
			int n(idPeer.size());
			if (!n) return -1;

			int i, id, sechit(1), maxhit(1);
			for (i = 0; i < n; ++i) {
				if (maxhit < idPeer[i].hit) {
					sechit = maxhit;
					maxhit = idPeer[i].hit;
					id     = idPeer[i].id;
				}
			}

			ratio = double(maxhit) / sechit;
			return id;
		}
	};
	using MatchedPtVec = std::vector<matched_point>;

protected:
	/* 参数 */
	double aimg_min_;			//< 约束: 定向点的中心距
	double diff_incl_max_;		//< 约束: 倾角最大偏差
	double diff_lnormal_max_;	//< 约束: 归一距离最大偏差
	int shape_count_min_;		//< 约束: 匹配单元最小数量
	int count_img_max_;			//< 约束: 图像系参与匹配的最大目标数
	int count_wcs_max_;			//< 约束: 世界系参与匹配的最大目标数
	double good_match_;			//< 约束: 匹配成功阈值

	/* 匹配项 */
	refcenter refwcs_;	//< 世界坐标中心, 量纲: 弧度
	ObjImgVec objimg_;	//< 图像坐标集合
	ObjWcsVec objwcs_;	//< 世界坐标集合

	double scale_low_;	//< 像元比列尺下限, 弧度/像素
	double scale_high_;	//< 像元比列尺上限
	double aimg_low_;	//< 阈值: 定向点的中心距, 图像坐标系
	double awcs_low_;	//< 阈值: 定向点的中心距, 世界坐标系

	int imgsample_;		//< 参与匹配的图像样本数量
	int wcssample_;		//< 参与匹配的世界样本数量
	WedgeShapeVec shapeimg_;	//< 图像匹配单元集合
	WedgeShapeVec shapewcs_;	//< 世界匹配单元集合
	MatchedPtVec matched_;	//< 匹配候选

public:
	/* 接口 */
	void SetGuessScale(double low, double high);
	/*!
	 * @brief 导入参与匹配的图像和世界坐标
	 * @param x   图像坐标, 量纲: 像素
	 * @param y   图像坐标, 量纲: 像素
	 * @param l   世界坐标, 量纲: 角度
	 * @param b   世界坐标, 量纲: 角度
	 */
	void BeginImportImageObject(int w, int h);
	void BeginImportWcsObject(double l, double b);
	void ImportImageObject(double x, double y, double flux);
	void ImportWcsObject(double l, double b, float mag);
	void CompleteImportImageObject();
	void CompleteImportWcsObjectr();
	/*!
	 * @brief 执行匹配流程
	 * @return
	 * 匹配结果
	 */
	bool DoMatch();

protected:
	/* 功能 */
	void sphere2plane(double l, double b, double &xi, double &eta);

	/*!
	 * @brief 计算图像坐标系两点线段的距离和倾角
	 * @param idc  中心ID
	 * @param id   指向ID
	 * @param len  线段长度
	 * @param incl 倾角, 量纲: 角度
	 */
	void calc_image_points(const int idc, const int id, double &len, double &incl);
	/*!
	 * @brief 计算世界坐标系两点线段的距离和倾角
	 * @param idc  中心ID
	 * @param id   指向ID
	 * @param len  线段长度
	 * @param incl 倾角, 量纲: 角度
	 */
	void calc_wcs_points(const int idc, const int id, double &len, double &incl);

	/*!
	 * @brief 依据参数尝试建立一个匹配单元
	 * @param idc    中心ID
	 * @param ido    指向ID
	 * @param angle  楔形夹角
	 * @param shape  建立的匹配单元
	 * @return
	 * 建立结果
	 * - true:  成功, 信息写入shape
	 * - false: 失败
	 */
	bool build_wedge_image(const int idc, const int ido, double angle, wedge_shape &shape);
	/*!
	 * @brief 以id为中心构建所有可能的匹配单元
	 * @param angle 匹配单元夹角, 量纲: 角度
	 * @return
	 * 构建结果
	 */
	bool build_wedge_image(double angle);

	/*!
	 * @brief 依据参数尝试建立一个匹配单元
	 * @param idc    中心ID
	 * @param ido    指向ID
	 * @param angle  楔形夹角
	 * @param shape  建立的匹配单元
	 * @return
	 * 建立结果
	 * - true:  成功, 匹配单元信息写入shape
	 * - false: 失败
	 */
	bool build_wedge_wcs(const int idc, const int ido, double angle, wedge_shape &shape);
	/*!
	 * @brief 以id为中心构建所有可能的匹配单元
	 * @param angle 匹配单元夹角, 量纲: 角度
	 * @return
	 * 构建结果
	 */
	bool build_wedge_wcs(double angle);

	/*!
	 * @brief 匹配图像系和世界系
	 * @param shapeImg  图像系匹配单元
	 * @param shapeWcs  世界系匹配单元
	 * @return
	 * 匹配结果
	 */
	bool match_wedge(const wedge_shape &shapeImg, const wedge_shape &shapeWcs);
};

#endif /* MATCHREFSYS_H_ */
