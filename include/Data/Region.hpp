/* 
 * Copyright (C) 2026 Arno Saxena
 *
 * Atlantis Majordomo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * File: Region.hpp
 */
 
// 304c89c8-6d3c-4586-b0c4-fad2e67b2f65
#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

/**
* @brief Data model for a region entity.
*/
class Region
{
public:
  Region(int xCoordinate,
        int yCoordinate,
         int zCoordinate,
        std::wstring regionType,
        std::wstring provinceName,
         bool containsSettlement,
    std::wstring settlementName,
         std::wstring settlementType,
        std::wstring peasantType,
         int peasantNumber,
         double wages = 0.0,
         int wagesMax = 0,
         int taxableIncome = 0,
         int month = 0,
         int year = 0,
         bool visited = false);

  Region(int xCoordinate,
         int yCoordinate,
         std::wstring regionType,
         std::wstring provinceName,
         bool containsSettlement,
         std::wstring settlementName,
         std::wstring settlementType,
         std::wstring peasantType,
         int peasantNumber,
         double wages = 0.0,
         int wagesMax = 0,
         int taxableIncome = 0,
         int month = 0,
         int year = 0,
         bool visited = false);
  virtual ~Region() = default;

  Region(const Region&) = default;
  Region& operator=(const Region&) = default;
  Region(Region&&) = default;
  Region& operator=(Region&&) = default;

  virtual int getXCoordinate() const;
  virtual int getYCoordinate() const;
  virtual int getZCoordinate() const;
  virtual int getMonth() const;
  virtual int getYear() const;
  virtual const std::wstring& getRegionType() const;
  virtual const std::wstring& getProvinceName() const;
  virtual bool getContainsSettlement() const;
  virtual const std::wstring& getSettlementName() const;
  virtual const std::wstring& getSettlementType() const;
  virtual const std::wstring& getPeasantType() const;
  virtual int getPeasantNumber() const;
  virtual double getWages() const;
  virtual int getWagesMax() const;
  int getWagesAfterOrders() const;
  virtual int getTaxableIncome() const;
  int getTaxableIncomeAfterOrders() const;
  virtual int getEntertainment() const;
  int getEntertainmentAfterOrders() const;
  virtual bool getVisited() const;
  virtual const std::vector<std::wstring>& getExitDirections() const;
  bool isOcean() const;
  virtual const std::map<std::wstring, int> &getResources() const;
  const std::map<std::wstring, int>& getResourcesAfterOrders() const;
  virtual const std::map<std::wstring, std::pair<int, int>>& getWanted() const;
  const std::map<std::wstring, std::pair<int, int>>& getWantedAfterOrders() const;
  virtual const std::map<std::wstring, std::pair<int, int>>& getForSale() const;
  const std::map<std::wstring, std::pair<int, int>>& getForSaleAfterOrders() const;
  virtual const std::wstring& getRegionReport() const;

  void setMonth(int month);
  void setYear(int year);
  void setZCoordinate(int zCoordinate);
  void setRegionType(std::wstring regionType);
  void setProvinceName(std::wstring provinceName);
  void setContainsSettlement(bool containsSettlement);
  void setSettlementName(std::wstring settlementName);
  void setSettlementType(std::wstring settlementType);
  void setPeasantType(std::wstring peasantType);
  void setPeasantNumber(int peasantNumber);
  void setWages(double wages);
  void setWagesMax(int wagesMax);
  void setWagesAfterOrders(int wagesAfterOrders);
  void setTaxableIncome(int taxableIncome);
  void setTaxableIncomeAfterOrders(int taxableIncomeAfterOrders);
  virtual void setEntertainment(int entertainment);
  void setEntertainmentAfterOrders(int entertainmentAfterOrders);
  void setVisited(bool visited);
  virtual void clearExitDirections();
  virtual void addExitDirection(std::wstring direction);
  virtual void setResources(std::map<std::wstring, int> resources);
  void setResourcesAfterOrders(std::map<std::wstring, int> resourcesAfterOrders);
  void setResource(std::wstring token, int amount);
  void setResourceAfterOrders(std::wstring token, int amount);
  virtual void setWanted(std::map<std::wstring, std::pair<int, int>> wanted);
  void setWantedAfterOrders(std::map<std::wstring, std::pair<int, int>> wantedAfterOrders);
  virtual void setForSale(std::map<std::wstring, std::pair<int, int>> forSale);
  void setForSaleAfterOrders(std::map<std::wstring, std::pair<int, int>> forSaleAfterOrders);
  void setWantedItem(std::wstring token, int amount, int price);
  void setWantedItemAfterOrders(std::wstring token, int amount, int price);
  void setForSaleItem(std::wstring token, int amount, int price);
  void setForSaleItemAfterOrders(std::wstring token, int amount, int price);
  virtual void setRegionReport(std::wstring regionReport);



private:
  int          xCoordinate_ { 0 };
  int          yCoordinate_ { 0 };
  int          zCoordinate_ { 1 };
  int          month_ { 0 };
  int          year_ { 0 };
  std::wstring regionType_;
  std::wstring provinceName_;
  bool         containsSettlement_ { false };
  std::wstring settlementName_;
  std::wstring settlementType_;
  std::wstring peasantType_;
  int          peasantNumber_ { 0 };
  double       wages_ { 0.0 };
  int          wagesMax_ { 0 };
  int          wagesAfterOrders_ { 0 };
  int          taxableIncome_{ 0 };
  int          taxableIncomeAfterOrders_{ 0 };
  int          entertainment_ { 0 };
  int          entertainmentAfterOrders_ { 0 };
  bool         visited_ { false };
  std::vector<std::wstring> exitDirections_;
  std::map<std::wstring, int> resources_;
  std::map<std::wstring, int> resourcesAfterOrders_;
  std::map<std::wstring, std::pair<int, int>> wanted_;
  std::map<std::wstring, std::pair<int, int>> wantedAfterOrders_;
  std::map<std::wstring, std::pair<int, int>> forSale_;
  std::map<std::wstring, std::pair<int, int>> forSaleAfterOrders_;
  std::wstring regionReport_;
};
