// File: scan_data_manager.cpp
#include "scan_data_manager.h"
#include <SD.h>

bool loadMeta(const char* metaPath, ScanConfig& cfg) {
    if (!SD.exists(metaPath)) return false;
    File f = SD.open(metaPath, FILE_READ);
    if (!f) return false;
    String line = f.readStringUntil('\n');
    f.close();

    // tokens: version,macCount,mac1...macN,segments,scans,featRSSI,featTOF,unix
    std::vector<String> toks;
    int start = 0;
    while (true) {
        int c = line.indexOf(',', start);
        if (c < 0) { toks.push_back(line.substring(start)); break; }
        toks.push_back(line.substring(start, c));
        start = c + 1;
    }
    if (toks.size() < 7) return false;

    int idx = 0;
    cfg.layoutVersion    = toks[idx++].toInt();
    int macCount         = toks[idx++].toInt();
    if ((int)toks.size() < 2 + macCount + 4) return false;

    cfg.anchorMacs.clear();
    for (int i = 0; i < macCount; ++i) {
        cfg.anchorMacs.push_back(toks[idx++]);
    }
    cfg.expectedSegments = toks[idx++].toInt();
    cfg.scansPerSegment  = toks[idx++].toInt();
    cfg.featureCountRSSI = toks[idx++].toInt();
    cfg.featureCountTOF  = toks[idx++].toInt();
    // ignore final timestamp token
    return true;
}

bool metaMatches(const ScanConfig& stored, const ScanConfig& current) {
    if (stored.layoutVersion    != current.layoutVersion)    return false;
    if (stored.expectedSegments != current.expectedSegments) return false;
    if (stored.scansPerSegment  != current.scansPerSegment)  return false;
    if (stored.featureCountRSSI != current.featureCountRSSI) return false;
    if (stored.featureCountTOF  != current.featureCountTOF)  return false;
    if (stored.anchorMacs.size() != current.anchorMacs.size()) return false;
    for (size_t i = 0; i < stored.anchorMacs.size(); ++i) {
        if (stored.anchorMacs[i] != current.anchorMacs[i]) return false;
    }
    return true;
}

bool isCSVComplete(const char* csvPath, const ScanConfig& cfg) {
    if (!SD.exists(csvPath)) return false;
    File f = SD.open(csvPath, FILE_READ);
    if (!f) return false;
    f.readStringUntil('\n');  // skip header

    int count = 0, needed = cfg.expectedSegments * cfg.scansPerSegment;
    while (f.available() && count < needed) {
        String line = f.readStringUntil('\n');
        if (line.length() > 2) ++count;
    }
    f.close();
    return (count >= needed);
}

void saveMeta(const char* metaPath,
              const ScanConfig& cfg,
              uint32_t lastScanUnix)
{
    File f = SD.open(metaPath, FILE_WRITE);
    if (!f) return;
    f.print(cfg.layoutVersion);   f.print(',');
    f.print(cfg.anchorMacs.size());
    for (auto &m : cfg.anchorMacs) {
        f.print(','); f.print(m);
    }
    f.print(','); f.print(cfg.expectedSegments);
    f.print(','); f.print(cfg.scansPerSegment);
    f.print(','); f.print(cfg.featureCountRSSI);
    f.print(','); f.print(cfg.featureCountTOF);
    f.print(','); f.println(lastScanUnix);
    f.close();
}


// Helper: split a CSV line into tokens
static void splitCSV(const String &line, std::vector<String> &out) {
  out.clear();
  int start = 0;
  while (true) {
    int c = line.indexOf(',', start);
    if (c < 0) {
      out.push_back(line.substring(start));
      return;
    }
    out.push_back(line.substring(start, c));
    start = c + 1;
  }
}

bool pruneIncompleteSegments(const char* csvPath,
                             const ScanConfig& cfg,
                             std::vector<int>& doneLocs) {
  if (!SD.exists(csvPath)) return false;
  File f = SD.open(csvPath, FILE_READ);
  if (!f) return false;

  // 1) Read header
  String header = f.readStringUntil('\n');

  // 2) First pass: count rows per location, stash all lines
  std::map<int,int> counts;
  std::vector<String> lines;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() < 3) continue;
    lines.push_back(line);

    // parse tokens
    std::vector<String> toks;
    splitCSV(line, toks);
    // location is at index = number of RSSI columns
    int loc = toks[cfg.anchorMacs.size()].toInt();
    counts[loc]++;
  }
  f.close();

  // 3) Determine fully done locations
  doneLocs.clear();
  for (int loc = 0; loc < cfg.expectedSegments; ++loc) {
    if (counts[loc] == cfg.scansPerSegment) {
      doneLocs.push_back(loc);
    }
  }

  // 4) Rewrite CSV to a temp file, only complete locations
  const char* tmp = "/.tmp.csv";
  File fout = SD.open(tmp, FILE_WRITE);
  if (!fout) return false;
  fout.println(header);
  for (auto &line : lines) {
    std::vector<String> toks;
    splitCSV(line, toks);
    int loc = toks[cfg.anchorMacs.size()].toInt();
    if (counts[loc] == cfg.scansPerSegment) {
      fout.println(line);
    }
  }
  fout.close();

  // 5) Replace original with pruned file
  SD.remove(csvPath);
  SD.rename(tmp, csvPath);
  return true;
}

bool shouldReuseScans(const char* metaPath,
                      const char* csvPath,
                      const ScanConfig& cfg) {
  // 1) metadata file must exist and match
  if (!SD.exists(metaPath))                    
   return false;
  ScanConfig stored;
  if (!loadMeta(metaPath, stored) || !metaMatches(stored, cfg))       
   return false;

  // 2) CSV file must exist and its header must match
  if (!SD.exists(csvPath) ||!verifyCSVFormat(csvPath)) 
    return false;

  // 3) CSV must have exactly expectedSegments*scansPerSegment rows
 // if (!isCSVComplete(csvPath, cfg)) 
    // return false;

  return true;
}

