/**
 * @class MatchRefsys 参考系匹配
 * @version 0.1
 * @date Sep 2020
 */

#include <algorithm>
#include "ADefine.h"
#include "MatchRefsys.h"

using namespace std;
using namespace AstroUtil;

MatchRefsys::MatchRefsys() {
	aimg_min_ = 50.0;
	diff_incl_max_ = 0.1;
	diff_lnormal_max_ = 0.002;
	shape_count_min_ = 10;
	count_img_max_ = 40;
	count_wcs_max_ = count_img_max_ * 3;
	good_match_ = 0.5;

	scale_low_ = scale_high_ = 0.0;
	aimg_low_ = 0.0;
	awcs_low_ = 0.0;

	imgsample_ = 0;
	wcssample_ = 0;
	matched_.resize(count_img_max_);
}

MatchRefsys::~MatchRefsys() {

}

void MatchRefsys::SetGuessScale(double low, double high) {
	scale_low_  = low * AS2R;
	scale_high_ = high * AS2R;
}

void MatchRefsys::BeginImportImageObject(int w, int h) {
	if ((aimg_low_ = sqrt(w * w + h * h) * 0.126) < aimg_min_) aimg_low_ = aimg_min_;
	objimg_.clear();
	shapeimg_.clear();
}

void MatchRefsys::BeginImportWcsObject(double l, double b) {
	refwcs_.x = l * D2R;
	refwcs_.y = b * D2R;
	objwcs_.clear();
	shapewcs_.clear();
}

void MatchRefsys::ImportImageObject(double x, double y, double flux) {
	if (flux > 1.0) {
		object_image obj;
		obj.x = x;
		obj.y = y;
		obj.brightness = short((20.0 - 2.5 * log10(flux)) * 1000.0);
		objimg_.push_back(obj);
	}
}

void MatchRefsys::ImportWcsObject(double l, double b, float mag) {
	object_wcs obj;
	obj.l = l * D2R;
	obj.b = b * D2R;
	obj.brightness = short(mag * 1000.0);
	sphere2plane(obj.l, obj.b, obj.x, obj.y);
	objwcs_.push_back(obj);
}

void MatchRefsys::CompleteImportImageObject() {
	/* 按照流量递减排序 */
	stable_sort(objimg_.begin(), objimg_.end(), [](const object_image& x1, const object_image& x2) {
		return (x1.brightness <= x2.brightness);
	});
	// 初始化候选匹配项
	imgsample_ = objimg_.size() > count_img_max_ ? count_img_max_ : objimg_.size();
	for (int i = 0; i < imgsample_; ++i) matched_[i].reset();
}

void MatchRefsys::CompleteImportWcsObjectr() {
	/* 按照星等递增排序 */
	stable_sort(objwcs_.begin(), objwcs_.end(), [](const object_wcs& x1, const object_wcs& x2) {
		return (x1.brightness <= x2.brightness);
	});
	wcssample_ = objwcs_.size() > count_wcs_max_ ? count_wcs_max_ : objwcs_.size();
}

bool MatchRefsys::DoMatch() {
	awcs_low_ = scale_low_ * aimg_low_;

	if (!build_wedge_image(60.0)) return false;
	if (!build_wedge_wcs(60.0)) return false;

	int n1(shapeimg_.size()), n2(shapewcs_.size()), n(0);
	int i, j;
	bool success(false);

	for (i = 0; i < n1; ++i) {
		wedge_shape& shapeimg = shapeimg_[i];
		for (j = 0; j < n2; ++j) {
			if (match_wedge(shapeimg, shapewcs_[j])) ++n;
		}
	}

	if (n) {
		int id;
		double ratio;

		for (i = 0, n = 0; i < imgsample_; ++i) {
			if ((id = matched_[i].get_maxhit(ratio)) >= 0 && ratio > 3.0) ++n;
		}
		success = n > int(imgsample_ * good_match_);
		if (success) {
			for (i = 0, n = 0; i < imgsample_; ++i) {
				if ((id = matched_[i].get_maxhit(ratio)) >= 0 && ratio > 3.0) {
					printf ("%4d %6.1f %6.1f | %4d %8.4f %8.4f | %4lu %4d\n",
							i, objimg_[i].x, objimg_[i].y,
							id, objwcs_[id].l * R2D, objwcs_[id].b * R2D,
							matched_[i].idPeer.size(), int(ratio));
				}
			}
		}
	}
	return success;
}

void MatchRefsys::sphere2plane(double l, double b, double &xi, double &eta) {
	double fract = sin(refwcs_.y) * sin(b) + cos(refwcs_.y) * cos(b) * cos(l - refwcs_.x);
	xi  = cos(b) * sin(l - refwcs_.x) / fract;
	eta = (cos(refwcs_.y) * sin(b) - sin(refwcs_.y) * cos(b) * cos(l - refwcs_.x)) / fract;
}

void MatchRefsys::calc_image_points(const int idc, const int id, double &len, double &incl) {
	double dx = objimg_[id].x - objimg_[idc].x;
	double dy = objimg_[id].y - objimg_[idc].y;
	len  = sqrt(dx * dx + dy * dy);
	incl = atan2(dy, dx) * R2D;
}

void MatchRefsys::calc_wcs_points(const int idc, const int id, double &len, double &incl) {
	double dx = objwcs_[id].x - objwcs_[idc].x;
	double dy = objwcs_[id].y - objwcs_[idc].y;
	len  = sqrt(dx * dx + dy * dy);
	incl = atan2(dy, dx) * R2D;
}

bool MatchRefsys::build_wedge_image(const int idc, const int ido, double angle, wedge_shape &shape) {
	int id;
	double low2 = aimg_low_ * aimg_low_;
	double len_orient, incl_orient, recip;
	double len, incl;
	double dx, dy;

	// 计算指向点的距离和倾角
	calc_image_points(idc, ido, len_orient, incl_orient);
	if (len_orient < aimg_low_) return false;
	recip = 1.0 / len_orient;
	// 遍历符合条件的图像点
	angle *= 0.5;
	for (id = 0; id < imgsample_; ++id) {
		if (!(id == idc || id == ido)) {
			dx = objimg_[id].x - objimg_[idc].x;
			dy = objimg_[id].y - objimg_[idc].y;
			len = dx * dx + dy * dy;
			if (len < low2) continue;
			incl = atan2(dy, dx) * R2D - incl_orient;
			if (fabs(incl) > angle) continue;

			wedge_item item;
			item.id      = id;
			item.incl    = incl;
			item.lnormal = sqrt(len) * recip;
			shape.items.push_back(item);
		}
	}
	if (shape.items.size() < 2) return false;
	// 填充其它信息
	shape.idCenter = idc;
	shape.idOrient = ido;
	shape.incl     = incl_orient;
	shape.len      = len_orient;

	return true;
}

bool MatchRefsys::build_wedge_wcs(const int idc, const int ido, double angle, wedge_shape &shape) {
	int id;
	double low2 = awcs_low_ * awcs_low_;
	double len_orient, incl_orient, recip;
	double len, incl;
	double dx, dy;

	// 计算指向点的距离和倾角
	calc_wcs_points(idc, ido, len_orient, incl_orient);
	if (len_orient < awcs_low_) return false;
	recip = 1.0 / len_orient;
	// 遍历符合条件的世界点
	angle *= 0.5;
	for (id = 0; id < wcssample_; ++id) {
		if (!(id == idc || id == ido)) {
			dx = objwcs_[id].x - objwcs_[idc].x;
			dy = objwcs_[id].y - objwcs_[idc].y;
			len = dx * dx + dy * dy;
			if (len < low2) continue;
			incl = atan2(dy, dx) * R2D - incl_orient;
			if (fabs(incl) > angle) continue;

			wedge_item item;
			item.id      = id;
			item.incl    = incl;
			item.lnormal = sqrt(len) * recip;
			shape.items.push_back(item);
		}
	}
	if (shape.items.size() < 2) return false;
	// 填充其它信息
	shape.idCenter = idc;
	shape.idOrient = ido;
	shape.incl     = incl_orient;
	shape.len      = len_orient;

	return true;
}

bool MatchRefsys::build_wedge_image(double angle) {
	int idc, ido;

	for (idc = 0; idc < imgsample_; ++idc) {
		for (ido = idc + 1; ido < imgsample_; ++ido) {
			wedge_shape shape;
			if (build_wedge_image(idc, ido, angle, shape)) shapeimg_.push_back(shape);
		}
	}
	return shapeimg_.size() >= shape_count_min_;
}

bool MatchRefsys::build_wedge_wcs(double angle) {
	int idc, ido;

	for (idc = 0; idc < wcssample_; ++idc) {
		for (ido = idc + 1; ido < wcssample_; ++ido) {
			wedge_shape shape;
			if (build_wedge_wcs(idc, ido, angle, shape)) shapewcs_.push_back(shape);
		}
	}
	return shapewcs_.size() >= shape_count_min_;
}

bool MatchRefsys::match_wedge(const wedge_shape &shapeImg, const wedge_shape &shapeWcs) {
	double scale = shapeWcs.len / shapeImg.len;
	if (scale < scale_low_ || scale > scale_high_) return false;

	const WedgeItemVec& items_img = shapeImg.items;
	const WedgeItemVec& items_wcs = shapeWcs.items;
	double incl, lnormal;
	int n1(items_img.size()), n2(items_wcs.size()), n0(0);
	int i, j, id;

	for (i = 0; i < n1; ++i) {
		id      = items_img[i].id;
		incl    = items_img[i].incl;
		lnormal = items_img[i].lnormal;
		for (j = 0; j < n2; ++j) {
			// 正方向角度偏差大于阈值: 角度正反向需单独判定...
			if (   fabs(incl - items_wcs[j].incl) > diff_incl_max_) continue;
			// 归一距离偏差大于阈值
			if (fabs(items_wcs[j].lnormal - lnormal) > diff_lnormal_max_) continue;
			// 加入候选匹配项
			++n0;
			matched_[id].add_point(items_wcs[j].id);
		}
	}

	// 中心点和定向点加入候选匹配项
	if (n0) {
		matched_[shapeImg.idCenter].add_point(shapeWcs.idCenter);
		matched_[shapeImg.idOrient].add_point(shapeWcs.idOrient);
	}

	return n0;
}
