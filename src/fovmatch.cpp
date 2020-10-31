/**
 * 测试星场与星表匹配算法
 * 命令行参数:
 * - CAT文件路径. CAT文件记录已提取星像的测量信息, 主要是三列:
 *   1. X
 *   2. Y
 *   3. Flux
 */
#include <stdio.h>
#include "ADefine.h"
#include "ACatTycho2.h"
#include "MatchRefsys.h"

using namespace AstroUtil;

bool isValidRA(double x) {
	return (x >= 0.0 && x < 360.0);
}

bool isValidDEC(double x) {
	return (x > -90.0 && x < 90.0);
}

int load_cat(int w, int h, const char* filepath, MatchRefsys& match) {
	FILE *fp = fopen(filepath, "r");
	if (!fp) return -1;
	char line[100];
	char seps[] = " \t";
	char *token;
	double x, y, flux;
	int n(0);

	match.BeginImportImageObject(w, h);
	while (!feof(fp)) {
		if (!fgets(line, 100, fp) || line[0] == '#') continue;
		token = strtok(line, seps); x    = atof(token);
		token = strtok(NULL, seps); y    = atof(token);
		token = strtok(NULL, seps); flux = atof(token);
		match.ImportImageObject(x, y, flux);
		++n;
	}
	match.CompleteImportImageObject();
	fclose(fp);

	return n;
}

bool load_refstar(double ra, double dec, double fov, ACatTycho2& cat, MatchRefsys& match) {
	int nstar, i;
	ptr_tycho2_elem stars;

	if (!cat.FindStar(ra, dec, fov * 0.5)) {
		printf ("faild to find reference stars\n");
		return false;
	}
	stars = cat.GetResult(nstar);
	if (nstar < 5) {
		printf ("reference stars [%d] are not enough\n", nstar);
		return false;
	}

	match.BeginImportWcsObject(ra, dec);
	for (i = 0; i < nstar; ++i) {
		match.ImportWcsObject(stars[i].ra * MAS2D, stars[i].spd * MAS2D - 90.0, stars[i].mag * 0.001);
	}
	match.CompleteImportWcsObjectr();

	return true;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf ("Usage:\n");
		printf ("\t fovmatch catfile_path\n");
		return -1;
	}

	// 图像与中心指向
	int wimg(4096), himg(4096);	// 图像宽度和高度
	double scale_low(11.0), scale_high(12.0); // 像元比例尺, 估计值, 角秒/像素
//	double rac(230.0), decc(-13.0);	// 中心视场指向, 估计值, 角度
	double rac(1000.0), decc(-15.0);	// 中心视场指向, 估计值, 角度
	double fov;	// 匹配视场, 角分
	MatchRefsys match;

	if (load_cat(wimg, himg, argv[1], match) < 5) {
		printf ("fail to load image catalog[%s] or objects is not enough\n", argv[1]);
		return -2;
	}

	// !! 约束: 像元比例尺
	if (scale_low < 0.1) scale_low = 0.1;
	if (scale_high / scale_low > 1.414) scale_high = scale_low * 1.414;
	match.SetGuessScale(scale_low, scale_high);

	// 参考星表
	ACatTycho2 tycho2;

	tycho2.SetPathRoot("/Users/lxm/Catalogue/tycho2/tycho2.dat");

	if (isValidRA(rac) && isValidDEC(decc)) {
		/* 当知道中心粗略指向时, 直接在其附近星场尝试匹配 */
		fov = (wimg >= himg ? wimg : himg) * scale_high * 1.414 / 60.0; // 对角线视场

		if (!load_refstar(rac, decc, fov, tycho2, match)) {
			printf ("failed to load catalog or refstar is not enough\n");
			return -3;
		}
		if (match.DoMatch()) {
			printf ("match succeed\n");
			printf ("result:\n");
		}
		else {
			printf ("match failed\n");
		}
	}
	else {
		// 当中心指向未知时, 全天盲匹配. 全天盲匹配耗时较长
		bool success(false);
		double step = (wimg <= himg ? wimg : himg) * scale_low * 0.5 / 3600.0;
		double stepr;
		int nzd, izd, iz(0);

		fov = (wimg > himg ? wimg : himg) * scale_high * 1.414 / 60.0; // 对角线视场
		nzd = int(180.001 / step);

		for (izd = 0, decc = -12.0; izd < nzd && !success; decc -= step, ++izd) {
			if (decc < -90.0) decc += 180.0;
			if ((90.0 - fabs(decc)) < fov * 0.2 / 60.0) stepr = 360.1;
			else stepr = step / cos(decc * D2R);
			printf ("try to solve field %d. dec = %8.4f, stepD = %.4f, stepR = %.4f\n",
					++iz, decc, step, stepr);
			for (rac = 0.0; rac < 360.0 && !success; rac += stepr) {
				printf ("\t rac = %8.4f\n", rac);
				success = load_refstar(rac, decc, fov, tycho2, match) && match.DoMatch();
			}
		}

		if (success) {
			// 输出匹配结果和残差
			printf ("match succeed\n");
			printf ("result:\n");
		}
		else {
			printf ("match failed\n");
		}
	}

	return 0;
}
