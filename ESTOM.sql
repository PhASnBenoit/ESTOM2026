-- phpMyAdmin SQL Dump
-- version 5.2.2
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Apr 03, 2026 at 09:22 AM
-- Server version: 10.11.15-MariaDB
-- PHP Version: 7.4.33

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `ESTOM`
--

-- --------------------------------------------------------

--
-- Table structure for table `BOM`
--

CREATE TABLE `BOM` (
  `ID` int(11) NOT NULL,
  `Progression` decimal(5,2) DEFAULT 0.00 COMMENT 'Points',
  `Remplissage` int(11) DEFAULT 0,
  `Couleur` tinyint(4) DEFAULT 0,
  `Joueur` varchar(20) DEFAULT 'Joueur' COMMENT 'Sert à rien, joueurs identifiés par Couleur',
  `Status` tinyint(4) DEFAULT 0,
  `Connected` tinyint(4) DEFAULT 0,
  `NbrCollision` tinyint(4) DEFAULT 0,
  `IPAddr` varchar(20) NOT NULL DEFAULT '192.168.0.11',
  `Collisions` text DEFAULT '\'No\'' COMMENT 'Sert a rien, NbrCollisions'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;

--
-- Dumping data for table `BOM`
--

INSERT INTO `BOM` (`ID`, `Progression`, `Remplissage`, `Couleur`, `Joueur`, `Status`, `Connected`, `NbrCollision`, `IPAddr`, `Collisions`) VALUES
(1, 0.00, 0, 3, 'Joueur', 0, 0, NULL, '192.168.4.69', '\'No\'');

-- --------------------------------------------------------

--
-- Table structure for table `Config`
--

CREATE TABLE `Config` (
  `Id` int(11) NOT NULL,
  `Options` tinyint(1) NOT NULL DEFAULT 0 COMMENT '0=OptionsA, 1=Options B',
  `PtsRecolte` int(11) NOT NULL DEFAULT 10,
  `NbrPAV` int(11) NOT NULL DEFAULT 1,
  `MalusCollision` varchar(45) NOT NULL DEFAULT '0',
  `NbrBOM_V` int(11) NOT NULL DEFAULT 5,
  `NbrBOM_J` int(11) NOT NULL DEFAULT 5,
  `NbrBOM_N` int(11) NOT NULL DEFAULT 5,
  `NbrBOM_B` int(11) NOT NULL DEFAULT 5,
  `Temps_restant` int(11) DEFAULT NULL COMMENT '????',
  `duree` int(11) NOT NULL DEFAULT 180,
  `Status` int(11) NOT NULL DEFAULT 0 COMMENT 'Indication partie',
  `Heure_depart` datetime DEFAULT NULL COMMENT '????',
  `password` varchar(5) NOT NULL DEFAULT '84300',
  `Luminosite` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- Dumping data for table `Config`
--

INSERT INTO `Config` (`Id`, `Options`, `PtsRecolte`, `NbrPAV`, `MalusCollision`, `NbrBOM_V`, `NbrBOM_J`, `NbrBOM_N`, `NbrBOM_B`, `Temps_restant`, `duree`, `Status`, `Heure_depart`, `password`, `Luminosite`) VALUES
(1, 0, 10, 3, '0.4', 1, 1, 1, 1, NULL, 180, 2, NULL, '84300', 0);

-- --------------------------------------------------------

--
-- Table structure for table `PAV`
--

CREATE TABLE `PAV` (
  `ID` int(11) NOT NULL,
  `Stockage` tinyint(4) DEFAULT NULL COMMENT 'Sert à rien, que le PAV doit savoir son état',
  `Couleur` tinyint(4) DEFAULT 1,
  `Connected` tinyint(4) DEFAULT 0,
  `IPAddr` varchar(20) NOT NULL DEFAULT '192.168.0.12',
  `Status` tinyint(4) DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;

--
-- Dumping data for table `PAV`
--

INSERT INTO `PAV` (`ID`, `Stockage`, `Couleur`, `Connected`, `IPAddr`, `Status`) VALUES
(1, NULL, 1, 0, '192.168.4.36', 0);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `BOM`
--
ALTER TABLE `BOM`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `IPAddr` (`IPAddr`);

--
-- Indexes for table `Config`
--
ALTER TABLE `Config`
  ADD PRIMARY KEY (`Id`);

--
-- Indexes for table `PAV`
--
ALTER TABLE `PAV`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `IPAddr` (`IPAddr`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `BOM`
--
ALTER TABLE `BOM`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT for table `PAV`
--
ALTER TABLE `PAV`
  MODIFY `ID` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=59;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
